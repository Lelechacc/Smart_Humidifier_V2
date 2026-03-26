/**
 * @file bsp_atomizer.c
 * @brief 【加湿器 V6.1 旗舰版】雾化驱动模块底层实现
 * @version 6.1
 * 
 * =================================================================================
 * 【硬件背景知识】：
 * 1. 雾化片原理：微孔雾化片需要高频的交变电信号来驱动其压电陶瓷产生超声波。
 * 2. 谐振点：绝大多数加湿器雾化片的谐振点在 108kHz (108,000Hz)。
 * 3. 驱动芯片：ESP32-C3 通过 GPIO3 输出 PWM 信号给 N-MOS 管，MOS 管控制电感产生谐振。
 * 
 * 【开发规范】：
 * - 每一行代码配有详尽中文注释。
 * - 所有的 LOG 打印均为中文，方便调试。
 * - 逻辑层级清晰：初始化 -> 功率设置 -> 安全停止。
 * =================================================================================
 */

#include "driver/ledc.h"    // 引入 ESP-IDF 官方 LED PWM 控制器驱动库
#include "esp_log.h"        // 引入官方日志库
#include "board_pins.h"     // 引入我们的“引脚宪法”文件
#include "bsp_atomizer.h"   // 引入本模块的头文件声明

// 定义本模块的日志标签，串口监控时会显示“驱动-雾化动力V6”
static const char *TAG = "驱动-雾化动力V6";

/**
 * @brief 初始化雾化驱动硬件
 * @details 采用 ESP32-C3 的 LEDC 外设，它可以产生不占用 CPU 的高精度硬件 PWM 信号。
 */
void bsp_atomizer_init(void)
{
    // --- 步骤 1：配置 LEDC 定时器 (PWM 的“心脏”) ---
    // 定时器决定了 PWM 的频率（多快）和分辨率（多细）
    ledc_timer_config_t atomizer_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE, // ESP32-C3 固定使用低速模式
        .timer_num        = LEDC_TIMER_0,        // 使用内部的第 0 号定时器
        .duty_resolution  = LEDC_TIMER_8_BIT,    // 设置占空比分辨率为 8 位 (范围 0-255)
                                                 // 8 位意味着我们可以把功率分成 256 个档位，非常直观
        .freq_hz          = 108000,              // 【核心】锁定频率为 108,000 Hz (108kHz)
        .clk_cfg          = LEDC_AUTO_CLK        // 让系统自动选择最稳定的内部时钟源
    };
    
    // 调用官方库函数，将上面的配置写入芯片寄存器
    esp_err_t timer_err = ledc_timer_config(&atomizer_timer);
    if (timer_err != ESP_OK) {
        ESP_LOGE(TAG, "定时器配置出错！错误码: %d", timer_err);
    }

    // --- 步骤 2：配置 LEDC 通道 (PWM 的“出口”) ---
    // 通道决定了 PWM 信号从哪一个引脚输出，以及初始功率
    ledc_channel_config_t atomizer_channel = {
        .speed_mode     = LEDC_LOW_SPEED_MODE, // 必须与定时器模式一致
        .channel        = LEDC_CHANNEL_0,      // 使用第 0 号通道
        .timer_sel      = LEDC_TIMER_0,        // 绑定刚才配置好的 0 号定时器
        .intr_type      = LEDC_INTR_DISABLE,   // 此时不需要使用中断，降低系统开销
        .gpio_num       = PIN_ATOMIZER,        // 【核心】绑定到原理图上的 GPIO 3 引脚
        .duty           = 0,                   // 【关键安全】初始功率设为 0，防止上电乱喷
        .hpoint         = 0                    // 脉冲起始相位设为 0
    };
    
    // 调用官方库函数，将通道配置写入芯片
    esp_err_t chan_err = ledc_channel_config(&atomizer_channel);
    if (chan_err != ESP_OK) {
        ESP_LOGE(TAG, "通道配置出错！错误码: %d", chan_err);
    }

    /* 
     * --- 步骤 3：上电安全双重保险 ---
     * 强制调用停止函数，并指定引脚输出电平为 0。
     * 这确保了即使刚才初始化产生了一点抖动，现在引脚也一定是安全的低电平。
     */
    ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);

    ESP_LOGI(TAG, "雾化驱动 108kHz 初始化成功！[引脚: GPIO 3, 分辨率: 8-bit]");
}

/**
 * @brief 动态调节雾化功率 (修改 PWM 占空比)
 * @param duty 目标占空比数值 (0 到 255 之间)
 */
static uint32_t current_set_duty = 999; // 记录当前设置的占空比

void bsp_atomizer_set_duty(uint32_t duty)
{
    // 1. 安全限流：绝对锁定在 115 (45%) 以下
    uint32_t safe_val = (duty > 135) ? 135 : duty;

    // 2. 【核心优化】：状态检查
    // 如果请求的功率和当前运行的功率一模一样，直接跳过，不准骚扰硬件波形！
    if (safe_val == current_set_duty) {
        return; 
    }

    // 3. 执行真正的更新
    if (safe_val == 0) {
        ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0); // 彻底关断
    } else {
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, safe_val);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    }

    current_set_duty = safe_val; // 更新记事本
    ESP_LOGW(TAG, "功率同步更新：[%ld]", safe_val);
}

/**
 * @brief 紧急停止雾化驱动
 * @details 相比于设置 duty 为 0，ledc_stop 更加彻底，它会直接关闭 PWM 生成器。
 */
void bsp_atomizer_stop(void)
{
    // 强制停止通道 0，并让 GPIO 3 保持在逻辑 0 (低电平)
    ledc_stop(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ESP_LOGW(TAG, "硬件反馈：雾化驱动已彻底紧急停止。");
}