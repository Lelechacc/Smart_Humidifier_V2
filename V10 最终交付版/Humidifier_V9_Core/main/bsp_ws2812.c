/**
 * @file bsp_ws2812.c
 * @brief 【智能加湿器 V6.1 旗舰版】WS2812B 幻彩灯底层驱动实现
 * @version 6.1
 * 
 * @details 
 * 【设计背景】：
 * WS2812B 是一种集成了控制电路的 RGB 灯珠，它对控制信号的时序要求极度严苛（纳秒级）。
 * 传统的 GPIO 翻转由于受 FreeRTOS 任务切换和 CPU 频率波动的影响，很难产生稳定的信号。
 * 
 * 【技术选型】：
 * 本模块采用了 ESP-IDF 官方推荐的 `led_strip` 驱动组件，其底层基于 ESP32-C3 内部的 
 * RMT (Remote Control) 硬件外设。RMT 就像一个“硬件信号发生器”，它一旦配置好，
 * 就可以在不占用 CPU 的情况下，发出极其精准的时序波形。
 * 
 * 【开发规范】：
 * 1. 所有的 LOG 打印均采用全中文，方便国内开发者快速定位问题。
 * 2. 所有的硬件初始化均带有错误检查机制 (esp_err_t)。
 */

#include "esp_log.h"
#include "led_strip.h"    // 引入官方灯条组件，需确保 idf_component.yml 已添加依赖
#include "board_pins.h"   // 引入引脚定义：PIN_RGB_LED 对应 GPIO6
#include "bsp_ws2812.h"   // 引入本模块头文件

// ==========================================
//              内部私有变量
// ==========================================

// 日志系统打印时使用的标签，用于在串口监控中过滤“幻彩灯”相关信息
static const char *TAG = "驱动-幻彩灯V6";

// 定义灯条句柄 (Handle)
// 这是一个指针，指向 RMT 硬件分配的内存资源，所有的后续操作(变色、熄灭)都靠这把“钥匙”
static led_strip_handle_t led_strip = NULL;

/**
 * @brief 初始化 WS2812B 硬件驱动
 * @details 该函数会向系统申请 RMT 硬件资源，并将 GPIO6 配置为数据输出模式。
 */
void bsp_ws2812_init(void)
{
    // 1. 【配置灯条硬件参数】
    led_strip_config_t strip_config = {
        .strip_gpio_num = PIN_RGB_LED,            // 绑定原理图上的 GPIO6
        .max_leds = 1,                            // 设置驱动的灯珠数量：1 颗
        .led_pixel_format = LED_PIXEL_FORMAT_GRB, // WS2812B 采用 GRB 编码格式
        .led_model = LED_MODEL_WS2812,             // 指定驱动模型为 WS2812 芯片
        .flags.invert_out = false,                 // 信号不需要逻辑取反
    };

    // 2. 【配置 RMT 硬件后端】
    // RMT 是 ESP32 系列特有的外设，它像是一台高速脉冲录音机
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,            // 使用默认的系统时钟源
        .resolution_hz = 10 * 1000 * 1000,         // 设置 10MHz 分辨率，即一个滴答为 100 纳秒
        .flags.with_dma = false,                   // 只有一颗灯，不需要动用 DMA (直接存储器访问)
    };

    // 3. 【正式创建并安装驱动】
    // 将上面的“硬件配置”和“后端配置”合体，向内核申请创建灯条实例
    esp_err_t err = led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip);
    
    // 4. 【结果检查与状态反馈】
    if (err == ESP_OK) {
        // 如果驱动安装成功，先强制清空一次灯珠状态(全灭)，防止上电随机亮色
        led_strip_clear(led_strip); 
        ESP_LOGI(TAG, "WS2812B 驱动成功启动！[已绑定 GPIO6]");
    } else {
        // 如果安装失败，打印具体的错误码，方便排查硬件资源是否被占用
        ESP_LOGE(TAG, "WS2812B 初始化失败！错误代码: %d", err);
    }
}

/**
 * @brief 设置灯珠的具体颜色
 * @param r 红色分量 (0 ~ 255)
 * @param g 绿色分量 (0 ~ 255)
 * @param b 蓝色分量 (0 ~ 255)
 * 
 * @warning 亮度警示：
 * 所有的 RGB 参数在 255 满格时会产生剧烈的白光，极其刺眼，建议在 main.c 逻辑层中将数值
 * 控制在 50 以下。这样出来的颜色更具有“马卡龙”质感，且不伤眼。
 */
void bsp_ws2812_set_color(uint8_t r, uint8_t g, uint8_t b)
{
    // 在操作前，必须确保句柄不为空(即驱动已成功初始化)
    if (led_strip) {
        /* 
         * 步骤 A: 设置像素数值
         * 此函数只是修改了内存里的颜色缓冲区，并不会立刻改变灯珠颜色。
         * 第一个参数是索引(从0开始)，我们只有一颗灯，所以固定为 0。
         */
        led_strip_set_pixel(led_strip, 0, r, g, b);

        /* 
         * 步骤 B: 刷新物理显示
         * 此函数会将内存里的 GRB 数值通过 RMT 外设转换成时序脉冲，发送给引脚。
         * 这时灯珠才会真正发生颜色改变。
         */
        led_strip_refresh(led_strip);
    } else {
        ESP_LOGW(TAG, "尝试设置颜色失败：灯条驱动尚未初始化！");
    }
}

/**
 * @brief 熄灭所有灯珠并释放总线
 * @details 用于在关机或进入省电模式时使用，彻底关断灯珠的功耗。
 */
void bsp_ws2812_clear(void)
{
    if (led_strip) {
        // 1. 调用官方库的清除函数 (将所有通道设为 0 并刷新)
        led_strip_clear(led_strip);
        ESP_LOGI(TAG, "硬件反馈：灯珠已成功熄灭。");
    }
}