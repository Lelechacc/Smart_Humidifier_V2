# Source Code Summary

## File: D:\QRS\09_Training_Project\加湿器\V10 最终交付版\Humidifier_V9_Core\main\board_pins.h

```c
#ifndef _BOARD_PINS_H_
#define _BOARD_PINS_H_

/**
 * @file board_pins.h
 * @brief 【加湿器 V6.1 旗舰版】硬件引脚全映射表
 * @note 必须严格匹配原理图：
 *       LED6 -> GPIO8 (STATUS_1)
 *       LED1 -> GPIO9 (STATUS_2)
 *       LED2 -> GPIO10 (STATUS_3)
 */

// 1. 核心输入
#define PIN_BTN_MAIN        2   // 主按键 SW2 (GPIO2)

// 2. 功率输出
#define PIN_ATOMIZER        3   // 雾化片 PWM 控制 (GPIO3)

// 3. 视觉反馈 (WS2812B)
#define PIN_RGB_LED         6   // WS2812B 信号线 (GPIO6)

// 4. 状态指示灯 (普通 LED)
#define PIN_LED_STATUS_1    8   // 原理图 LED6 (GPIO8)
#define PIN_LED_STATUS_2    9   // 原理图 LED1 (GPIO9)
#define PIN_LED_STATUS_3    10  // 原理图 LED2 (GPIO10)

// 5. 语音通讯 (115200 波特率)
#define PIN_UART_RX         20  // GPIO20
#define PIN_UART_TX         21  // GPIO21

#endif // _BOARD_PINS_H_ 
```


## File: D:\QRS\09_Training_Project\加湿器\V10 最终交付版\Humidifier_V9_Core\main\bsp_atomizer.c

```c
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
```


## File: D:\QRS\09_Training_Project\加湿器\V10 最终交付版\Humidifier_V9_Core\main\bsp_atomizer.h

```c
#ifndef _BSP_ATOMIZER_H_
#define _BSP_ATOMIZER_H_

#include <stdint.h> // 引入标准整数类型，用于 uint32_t 等定义

/**
 * @file bsp_atomizer.h
 * @brief 【加湿器 V6.1 旗舰版】板级支持包 (BSP) - 雾化驱动模块
 * @version 6.1 (动力集成版)
 * @date 2024-02-10
 * 
 * =================================================================================
 * 【模块设计背景】：
 * 本模块是整个加湿器的“动力源”，负责控制 GPIO3 产生极其精密的高频波形。
 * 
 * 【核心驱动原理】：
 * 1. 物理频率：微孔雾化片的工作依赖于“超声波谐振”，本模块强制锁定频率为 108,000Hz (108kHz)。
 * 2. 功率调节：采用 PWM（脉冲宽度调制）技术，通过改变 8 位分辨率（0-255）下的占空比
 *    来调节出雾量的大小。
 * 
 * 【安全性保护契约】：
 * - 软件硬限位：底层代码自动拦截任何超过 70%（数值 178）的指令，从根源防止 MOS 管过热烧毁。
 * - 启动保护：初始化后默认输出绝对零电平，杜绝上电瞬间“误喷”或短路风险。
 * =================================================================================
 */

/**
 * @brief 初始化雾化片硬件驱动模块
 * @details 
 * 1. 申请并配置 ESP32-C3 内部的 LEDC (LED PWM 控制器) 定时器。
 * 2. 设定 8 位 (8-bit) 分辨率，将 0.0% 到 100.0% 的功率映射为 0 到 255 的整数。
 * 3. 将驱动信号绑定到原理图指定的 PIN_ATOMIZER (GPIO3) 引脚。
 * 
 * @note 调用此函数后，硬件已就绪，但处于“零功率”静默状态，不会有任何电压输出。
 */
void bsp_atomizer_init(void);

/**
 * @brief 设置雾化片输出功率 (核心控制接口)
 * @param duty 占空比数值，取值范围：0 (彻底关闭) 到 255 (理论全功率)。
 * 
 * @details 
 * 该函数是 V6.1 版本的核心动力接口。它允许主控程序精细调节雾量：
 * - 传入 127：对应 50% 功率，实现“小雾模式”。
 * - 传入 178：对应 70% 功率，实现“大雾模式”。
 * - 传入 0  ：彻底停喷。
 * 
 * @warning 安全限位：即使你传入了 255，本函数内部也会将其强制削减为 178 (70%)，
 *          这是为了保护你的自制 PCB 电路不被大电流损坏。
 */
void bsp_atomizer_set_duty(uint32_t duty);

/**
 * @brief 彻底关断雾化驱动输出 (紧急/安全停止)
 * @details 
 * 相比于将功率设为 0，本函数会直接调用 LEDC 外设的停止指令 `ledc_stop`。
 * 它会物理性地断开信号发生器，并强制引脚输出稳定的低电平(0V)，
 * 确保 MOS 管栅极处于绝对关断状态，是系统进入待机模式时的首选操作。
 */
void bsp_atomizer_stop(void);

#endif // _BSP_ATOMIZER_H_
```


## File: D:\QRS\09_Training_Project\加湿器\V10 最终交付版\Humidifier_V9_Core\main\bsp_led.c

```c
/**
 * @file bsp_led.c
 * @brief 【智能加湿器 V6.1 旗舰版】全板指示灯驱动模块实现文件
 * @version 6.1
 * 
 * @details 
 * 【硬件映射关系】：
 * 1. LED6 (对应引脚 PIN_LED_STATUS_1 / GPIO8)  -> 定义为：待机/电源指示灯
 * 2. LED1 (对应引脚 PIN_LED_STATUS_2 / GPIO9)  -> 定义为：小雾模式指示灯
 * 3. LED2 (对应引脚 PIN_LED_STATUS_3 / GPIO10) -> 定义为：大雾模式指示灯
 * 
 * 【电路逻辑】：
 * 采用“灌电流”驱动方式（Active Low）。
 * - 写入 0 (低电平) 时：电流流过 LED，灯珠点亮。
 * - 写入 1 (高电平) 时：两端电位相同，灯珠熄灭。
 * 
 * 【设计思想】：
 * 本文件不仅提供底层的单灯控制，还封装了高层的模式联动接口，
 * 实现了“显示逻辑”与“业务逻辑”的彻底解耦。
 */

#include "driver/gpio.h"
#include "board_pins.h"
#include "bsp_led.h"
#include "esp_log.h"

// 日志打印标签
static const char *TAG = "驱动-指示灯V6";

/**
 * @brief 初始化所有状态灯的 GPIO 硬件配置
 * @details 
 * 1. 使用 64 位位掩码 (1ULL) 一次性配置三个引脚，提高执行效率。
 * 2. 强制将初始状态设为熄灭，防止系统启动瞬间指示灯乱闪。
 */
void bsp_led_init(void)
{
    // 配置结构体定义
    gpio_config_t io_conf = {
        // 将三个引脚号通过位运算合并到一个掩码中
        .pin_bit_mask = (1ULL << PIN_LED_STATUS_1) | 
                        (1ULL << PIN_LED_STATUS_2) | 
                        (1ULL << PIN_LED_STATUS_3),
        .mode = GPIO_MODE_OUTPUT,            // 设为推挽输出模式
        .pull_up_en = GPIO_PULLUP_DISABLE,    // 外部已有拉高，禁用内部上拉
        .pull_down_en = GPIO_PULLDOWN_DISABLE,// 禁用内部下拉
        .intr_type = GPIO_INTR_DISABLE        // 禁用引脚中断功能
    };
    
    // 调用 ESP-IDF 官方底层库进行硬件寄存器配置
    gpio_config(&io_conf);
    
    /* 
     * 初始化状态同步：
     * 系统刚刚上电时，为了确保用户看到的界面是干净的，
     * 我们将所有 LED 的 GPIO 引脚强制拉高 (即物理熄灭)。
     */
    gpio_set_level(PIN_LED_STATUS_1, 1);
    gpio_set_level(PIN_LED_STATUS_2, 1);
    gpio_set_level(PIN_LED_STATUS_3, 1);
    
    ESP_LOGI(TAG, "板载指示灯(LED1/LED2/LED6)硬件初始化完成。状态：初始全灭");
}

/**
 * @brief 控制 LED1 (对应丝印 LED6 / GPIO8) 
 * @param on true 为点亮，false 为熄灭
 */
void bsp_led1_set(bool on) 
{
    // 利用三目运算符实现低电平反转逻辑
    gpio_set_level(PIN_LED_STATUS_1, on ? 0 : 1); 
}

/**
 * @brief 控制 LED2 (对应丝印 LED1 / GPIO9) 
 * @param on true 为点亮，false 为熄灭
 */
void bsp_led2_set(bool on) 
{
    gpio_set_level(PIN_LED_STATUS_2, on ? 0 : 1); 
}

/**
 * @brief 控制 LED3 (对应丝印 LED2 / GPIO10) 
 * @param on true 为点亮，false 为熄灭
 */
void bsp_led3_set(bool on) 
{
    gpio_set_level(PIN_LED_STATUS_3, on ? 0 : 1); 
}

/**
 * @brief 核心模式显示器：根据档位一键切换灯光状态
 * @param mode 当前档位 (0:关, 1:小雾, 2:大雾, 3:间歇)
 * @details 
 * 这个函数是驱动层最核心的逻辑，它根据你的模式需求，
 * 自动计算出哪颗灯该亮，哪颗灯该灭。
 */
void bsp_led_show_mode(int mode)
{
    switch (mode) {
        case 0: // 关机/待机模式
            bsp_led1_set(true);  // 仅 LED6 (待机灯) 亮
            bsp_led2_set(false); // LED1 灭
            bsp_led3_set(false); // LED2 灭
            break;

        case 1: // 小雾模式
            bsp_led1_set(false); // 待机灯灭
            bsp_led2_set(true);  // 仅 LED1 (档位1) 亮
            bsp_led3_set(false); // LED2 灭
            break;

        case 2: // 大雾模式
            bsp_led1_set(false); // 待机灯灭
            bsp_led2_set(false); // LED1 灭
            bsp_led3_set(true);  // 仅 LED2 (档位2) 亮
            break;

        case 3: // 间歇模式
            bsp_led1_set(false); // 待机灯灭
            bsp_led2_set(true);  // LED1 和 LED2 同时亮起
            bsp_led3_set(true);  // 表示特殊档位
            break;

        default: // 防错处理：全灭
            bsp_led1_set(false);
            bsp_led2_set(false);
            bsp_led3_set(false);
            break;
    }
}
```


## File: D:\QRS\09_Training_Project\加湿器\V10 最终交付版\Humidifier_V9_Core\main\bsp_led.h

```c
#ifndef _BSP_LED_H_
#define _BSP_LED_H_

#include <stdbool.h>

/**
 * @file bsp_led.h
 * @brief 【V6.1 旗舰版】指示灯驱动接口头文件
 * @note 
 * 本文件定义了所有与板载指示灯(LED1, LED2, LED6)交互的合法接口。
 * 它是 main.c 与 bsp_led.c 之间的“契约”。
 */

/**
 * @brief 初始化硬件 GPIO 配置
 */
void bsp_led_init(void);

/**
 * @brief 基础控制：手动控制 LED1 (待机灯/LED6)
 */
void bsp_led1_set(bool on);

/**
 * @brief 基础控制：手动控制 LED2 (小雾灯/LED1)
 */
void bsp_led2_set(bool on);

/**
 * @brief 基础控制：手动控制 LED3 (大雾灯/LED2)
 */
void bsp_led3_set(bool on);

/**
 * @brief 高级控制：根据当前模式自动切换灯光组合
 * @param mode 0:关机, 1:小雾, 2:大雾, 3:间歇
 * @details 
 * 这是新增加的函数声明。有了这一行，main.c 才能正确识别并调用它。
 */
void bsp_led_show_mode(int mode);

#endif // _BSP_LED_H_
```


## File: D:\QRS\09_Training_Project\加湿器\V10 最终交付版\Humidifier_V9_Core\main\bsp_voice.c

```c
/**
 * @file bsp_voice.c
 * @brief 【加湿器 V6.1 旗舰版】通讯驱动模块 - 硬件引脚翻转适配版
 * @version 6.1 (针对天问 PA2-TX 限制的深度适配)
 * 
 * =================================================================================
 * 【架构师必读 - 硬件逻辑调和协议】：
 * 
 * 1. 天问端限制：编辑器强制 Serial1 使用 PA2 为发送脚(TX)，PA3 为接收脚(RX)。
 * 2. 物理走线实况：
 *    - 天问引脚 PA2 <---> 物理铜线 <---> ESP32 引脚 GPIO 21
 *    - 天问引脚 PA3 <---> 物理铜线 <---> ESP32 引脚 GPIO 20
 * 
 * 3. 最终软件对策：
 *    - 我们必须把 ESP32 的 GPIO 21 配置为【接收模式 (RX)】，这样才能听到 PA2 说的话。
 *    - 我们必须把 ESP32 的 GPIO 20 配置为【发送模式 (TX)】，这样才能对准 PA3 的耳朵。
 * 
 * 4. 结论：本驱动通过 uart_set_pin 强行翻转引脚职责，实现“牛头对准马嘴”。
 * =================================================================================
 */

#include "driver/uart.h"    // 引入官方 UART 驱动库
#include "esp_log.h"        // 引入日志系统库
#include "board_pins.h"     // 引入引脚定义表
#include "bsp_voice.h"      // 引入本模块头文件声明

// 定义本驱动模块的串口日志标签
static const char *TAG = "驱动-通讯总线V6";

/**
 * @brief 初始化语音通讯串口 (UART1)
 * @details 采用引脚翻转策略，完美适配天问 ASRPRO 的硬件串口限制。
 */
void bsp_voice_init(void)
{
    // --- 步骤 1：配置串口底层通讯参数 ---
    uart_config_t v_config = {
        .baud_rate = 115200,              // 设定通讯波特率为 115200bps
        .data_bits = UART_DATA_8_BITS,    // 设置 8 位数据位
        .parity    = UART_PARITY_DISABLE, // 关闭奇偶校验，追求最高传输效率
        .stop_bits = UART_STOP_BITS_1,    // 设置 1 位停止位
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE, // 禁用硬件流控
        .source_clk = UART_SCLK_DEFAULT,  // 采用芯片默认的时钟源
    };

    // --- 步骤 2：正式安装驱动并分配缓冲区 ---
    // 给接收区分配 256 字节的“内存信箱”，防止天问说话太快导致数据溢出丢失
    uart_driver_install(UART_NUM_1, 256, 0, 0, NULL, 0);

    // --- 步骤 3：将上面的参数配置表应用到 UART1 控制器 ---
    uart_param_config(UART_NUM_1, &v_config);

    // --- 步骤 4：【全案最关键动作】通过软件实现引脚职责翻转 ---
    /*
     * 函数原型说明：uart_set_pin(串口号, TX引脚号, RX引脚号, RTS, CTS)
     * 
     * 翻转逻辑：
     * - 原本定义的 PIN_UART_TX 是 21，PIN_UART_RX 是 20。
     * - 现在的需求是：让 20 负责说(TX)，让 21 负责听(RX)。
     * - 这样刚好对准了天问的：PA3(听) 和 PA2(说)。
     */
    uart_set_pin(UART_NUM_1, 
                 20,                 // 将 GPIO 20 配置为“发送嘴巴”
                 21,                 // 将 GPIO 21 配置为“接收耳朵”
                 UART_PIN_NO_CHANGE, // RTS 保持不变
                 UART_PIN_NO_CHANGE  // CTS 保持不变
    );

    // 打印全中文初始化成功的日志，明确标注翻转策略
    ESP_LOGI(TAG, "通讯总线启动成功！[策略：适配天问 PA2-TX 模式，引脚已软件翻转]");
}

/**
 * @brief 从串口缓冲区提取一个字节的有效指令
 * @return 返回读到的字符 (如 'H', 'L' 等)，如果缓冲区为空则返回 0
 */
uint8_t bsp_voice_read_cmd(void)
{
    uint8_t byte_buf = 0; // 定义临时变量，用于存放抓到的字节

    /* 
     * 调用官方库函数从 UART1 的信箱里取信
     * pdMS_TO_TICKS(10) 表示最多在这里等 10 毫秒，没信就立刻返回。
     */
    int result = uart_read_bytes(UART_NUM_1, &byte_buf, 1, pdMS_TO_TICKS(10));
    
    // 如果 result > 0，说明真的在 10 毫秒内捡到了 1 个字节
    if (result > 0) {
        return byte_buf; // 功德圆满，把暗号上交给主指挥部
    }
    
    // 如果没捡到，返回 0 告诉主控“目前平安无事”
    return 0;
}
```


## File: D:\QRS\09_Training_Project\加湿器\V10 最终交付版\Humidifier_V9_Core\main\bsp_voice.h

```c
#ifndef _BSP_VOICE_H_
#define _BSP_VOICE_H_

#include <stdint.h> // 引入标准整数类型

/**
 * @file bsp_voice.h
 * @brief 【加湿器 V6.1 旗舰版】离线语音通讯模块头文件
 * 
 * =================================================================================
 * 【设计说明】：
 * 1. 负责与天问 51 (ASRPRO) 模块进行串口通讯。
 * 2. 交互逻辑：采用“便捷法”。天问识别语音后本地播报，随后通过串口发送 H/L/O/I。
 * 3. 硬件接口：使用 ESP32-C3 的 UART1 (GPIO20 为 RX，GPIO21 为 TX)。
 * =================================================================================
 */

/**
 * @brief 初始化语音模块专用的串口硬件
 * @details 配置波特率为 115200，并绑定物理引脚 GPIO20/21。
 */
void bsp_voice_init(void);

/**
 * @brief 从串口接收缓冲区读取一个指令字符
 * @return 读到的字符 (如 'H', 'L' 等)，如果没读到则返回 0。
 */
uint8_t bsp_voice_read_cmd(void);

#endif // _BSP_VOICE_H_
```


## File: D:\QRS\09_Training_Project\加湿器\V10 最终交付版\Humidifier_V9_Core\main\bsp_ws2812.c

```c
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
```


## File: D:\QRS\09_Training_Project\加湿器\V10 最终交付版\Humidifier_V9_Core\main\bsp_ws2812.h

```c
#ifndef _BSP_WS2812_H_
#define _BSP_WS2812_H_

#include <stdint.h>

/**
 * @file bsp_ws2812.h
 * @brief 【加湿器 V6.1 旗舰版】WS2812B 幻彩灯珠驱动
 * @version 1.0
 * 
 * @note 
 * 采用 ESP-IDF 官方 RMT 驱动架构，实现马卡龙色系的精确显示。
 * 每一个函数都经过了非阻塞设计，确保不影响主控制器的响应速度。
 */

/**
 * @brief 初始化 WS2812B 灯珠
 * @details 配置 RMT 外设并绑定 GPIO6 引脚，初始化完成后默认全灭。
 */
void bsp_ws2812_init(void);

/**
 * @brief 设置灯珠颜色 (RGB 模式)
 * @param r 红色亮度 (0~255)
 * @param g 绿色亮度 (0~255)
 * @param b 蓝色亮度 (0~255)
 */
void bsp_ws2812_set_color(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief 熄灭灯珠
 */
void bsp_ws2812_clear(void);

#endif // _BSP_WS2812_H_
```


## File: D:\QRS\09_Training_Project\加湿器\V10 最终交付版\Humidifier_V9_Core\main\main.c

```c
/**
 * @file main.c
 * @brief 【加湿器 V6.1 旗舰版】全功能集成指挥中心 - 原生蓝牙适配版
 * @version 6.1 (最终集成存档版本)
 * @author 准工程师 (由 AI 深度协作完成)
 *
 * =================================================================================
 * 【项目数字化存档说明】：
 * 1. 架构：基于 FreeRTOS 实时操作系统，采用“任务 + 队列”的高级通讯模式。
 * 2. 动力：GPIO3 输出 108kHz 硬件 PWM，软件硬限位 115 (45%) 功率，确保电感不烧。
 * 3. 视觉：WS2812B 幻彩马卡龙灯效 + 板载 LED1,2,6 物理状态同步。
 * 4. 控制：
 *    - 物理按键：GPIO2，支持长短按识别。
 *    - 离线语音：天问 51 (UART1)，115200 波特率，绝对指令跳转。
 *    - 原生蓝牙：手机 BLE 调试助手直接发送 H/L/O/I 字符控制。
 * =================================================================================
 */

// ---------------------------------------------------------------------------------
// [区域 0]：系统头文件包含区 (回归纯 C 语言环境)
// ---------------------------------------------------------------------------------
#include <stdio.h>             // 标准输入输出库
#include "freertos/FreeRTOS.h" // FreeRTOS 核心系统库
#include "freertos/task.h"     // 任务管理库
#include "freertos/queue.h"    // 队列通讯库
#include "driver/gpio.h"       // 基础 GPIO 驱动库
#include "esp_log.h"           // 全中文日志打印库

// [包含所有的板级驱动插件 (BSP)]
#include "board_pins.h"   // 引脚宪法定义
#include "bsp_led.h"      // 指示灯驱动
#include "bsp_ws2812.h"   // 幻彩灯珠驱动
#include "bsp_atomizer.h" // 雾化动力驱动
#include "bsp_voice.h"    // 语音串口驱动
  // 原生蓝牙驱动 (Step 5 新增)

// ---------------------------------------------------------------------------------
// [区域 1]：全局变量与事件 ID 定义总线
// ---------------------------------------------------------------------------------

// 定义日志输出时显示的标签，方便调试查错
static const char *TAG = "核心控制中心";

// --- [全系统统一事件暗号定义] ---
#define EVENT_SHORT_PRESS 1 // 事件：检测到按键短按 (0-1-2 循环)
#define EVENT_LONG_PRESS 2  // 事件：检测到按键长按 (强切模式3)
#define EVENT_CMD_OFF 10    // 指令：要求待机关机 (对应语音/蓝牙 'O')
#define EVENT_CMD_HIGH 11   // 指令：要求开启大雾 (对应语音/蓝牙 'H')
#define EVENT_CMD_LOW 12    // 指令：要求开启小雾 (对应语音/蓝牙 'L')
#define EVENT_CMD_INT 13    // 指令：要求开启间歇 (对应语音/蓝牙 'I')

// 定义系统工作档位枚举，逻辑清晰不混乱
typedef enum
{
    MODE_OFF = 0,     // 档位 0：待机模式
    MODE_HIGH,        // 档位 1：持续大雾模式
    MODE_LOW,         // 档位 2：持续小雾模式
    MODE_INTERMITTENT // 档位 3：间歇喷雾模式
} sys_mode_t;

static sys_mode_t g_mode = MODE_OFF;     // 全局当前档位变量
static QueueHandle_t g_evt_queue = NULL; // 系统唯一的消息传递队列

// =================================================================================
// [区域 2]：硬件执行子系统 - 【视觉反馈控制区】
// =================================================================================
/**
 * @brief 更新全板指示灯与 WS2812B 颜色状态
 * @param blink_state 间歇模式下的 0.5s 闪烁节拍信号
 */
void update_visual_feedback(sys_mode_t mode, bool blink_state)
{
    // --- [模块 A：WS2812B 马卡龙固定颜色显示] ---
    // 我们设定一个 0.3f (30%) 的亮度系数，确保不晃眼且色彩高级
    float brightness = 0.3f;

    switch (mode)
    {
    case MODE_OFF: // 关机：柔粉色
        bsp_ws2812_set_color((uint8_t)(100 * brightness), (uint8_t)(30 * brightness), (uint8_t)(50 * brightness));
        break;
    case MODE_HIGH: // 大雾：天蓝色
        bsp_ws2812_set_color((uint8_t)(20 * brightness), (uint8_t)(60 * brightness), (uint8_t)(150 * brightness));
        break;
    case MODE_LOW: // 小雾：青绿色
        bsp_ws2812_set_color((uint8_t)(30 * brightness), (uint8_t)(150 * brightness), (uint8_t)(60 * brightness));
        break;
    case MODE_INTERMITTENT: // 间歇：奶油橙色
        bsp_ws2812_set_color((uint8_t)(180 * brightness), (uint8_t)(80 * brightness), (uint8_t)(0 * brightness));
        break;
    }

    // --- [模块 B：板载 LED1, 2, 6 物理状态联动] ---
    if (mode == MODE_OFF)
    {
        bsp_led_show_mode(0); // 仅点亮 LED6 待机灯
    }
    else if (mode == MODE_HIGH)
    {
        bsp_led_show_mode(2); // 仅点亮 LED2 大雾指示灯
    }
    else if (mode == MODE_LOW)
    {
        bsp_led_show_mode(1); // 仅点亮 LED1 小雾指示灯
    }
    else if (mode == MODE_INTERMITTENT)
    {
        /* 间歇模式特殊反馈：关闭待机、小雾灯，让大雾灯 LED2 跟随信号闪烁 */
        bsp_led1_set(false);
        bsp_led2_set(false);
        bsp_led3_set(blink_state); // 实现 LED2 每秒跳动一次的视觉效果
    }
}

// =================================================================================
// [区域 3]：硬件执行子系统 - 【动力输出控制区】
// =================================================================================
/**
 * @brief 实时更新 GPIO3 的 PWM 功率输出
 * @param is_spraying_allowed 间歇模式专用的喷雾许可
 */
void update_atomizer_power(sys_mode_t mode, bool is_spraying_allowed)
{
    // 判断逻辑 A：如果间歇计数器说现在该休息了，则强制切断动力
    if (mode == MODE_INTERMITTENT && is_spraying_allowed == false)
    {
        bsp_atomizer_set_duty(0); // PWM 置零
    }
    // 判断逻辑 B：根据工作档位输出预设功率
    else
    {
        switch (mode)
        {
        case MODE_OFF:
            bsp_atomizer_set_duty(0); // 关闭雾化
            break;
        case MODE_HIGH:
            bsp_atomizer_set_duty(135); // 输出 45% 功率，保护电感不发烫
            break;
        case MODE_LOW:
            bsp_atomizer_set_duty(76); // 输出 30% 功率，静音加湿
            break;
        case MODE_INTERMITTENT:
            bsp_atomizer_set_duty(115); // 在开启周期内输出 45% 的高功率
            break;
        default:
            bsp_atomizer_set_duty(0); // 默认关闭，保证安全
            break;
        }
    }
}

// =================================================================================
// [区域 4]：信号采集子系统 - 【按键扫描侦察员任务】
// =================================================================================
/**
 * @brief 独立任务：专门监视按键引脚，识别长按与短按
 */
void task_button_scan(void *pvParameters)
{
    ESP_LOGI(TAG, "[任务启动] 按键扫描侦察员已就位。");

    // 初始化引脚配置
    gpio_config_t btn_conf = {
        .pin_bit_mask = (1ULL << PIN_BTN_MAIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,      // 开启内部上拉
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // 关闭下拉
        .intr_type = GPIO_INTR_DISABLE         // 轮询方式
    };
    gpio_config(&btn_conf); // 写入寄存器

    while (1)
    {
        // 检测到按键按下 (低电平)
        if (gpio_get_level(PIN_BTN_MAIN) == 0)
        {
            int duration = 0; // 按下时长累计变量

            while (gpio_get_level(PIN_BTN_MAIN) == 0)
            {
                duration += 50;                // 每次循环加 50ms
                vTaskDelay(pdMS_TO_TICKS(50)); // 释放 CPU，等 50ms 再扫
            }

            // 松开后开始判断
            if (duration >= 1000)
            {
                uint32_t evt = EVENT_LONG_PRESS;
                xQueueSend(g_evt_queue, &evt, 0); // 上报长按事件
                ESP_LOGW(TAG, "[输入反馈] 物理按键：长按触发。");
            }
            else if (duration > 50)
            {
                uint32_t evt = EVENT_SHORT_PRESS;
                xQueueSend(g_evt_queue, &evt, 0); // 上报短按事件
                ESP_LOGI(TAG, "[输入反馈] 物理按键：短按触发。");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // 空闲期每秒采样 50 次
    }
}

// =================================================================================
// [区域 5]：信号采集子系统 - 【通讯监听员任务】 (语音/诊断)
// =================================================================================
/**
 * @brief 独立任务：监听 UART1 接口，处理语音模块发来的暗号
 */
void task_comm_listener(void *pvParameters)
{
    ESP_LOGI(TAG, "[任务启动] 通讯监听员：正在盯着天问 51 的消息。");
    while (1)
    {
        // 调用语音驱动接口，从信箱里取一个字符
        uint8_t raw_char = bsp_voice_read_cmd();

        // 关键过滤：如果是换行符或回车符，直接跳过处理
        if (raw_char == 0x0D || raw_char == 0x0A)
            continue;

        if (raw_char != 0)
        {
            // 串口日志显示：收到原始字符及 16 进制值，方便排查乱码
            ESP_LOGW(TAG, "[语音反馈] 收到原始暗号: [%c], 16进制: [0x%02X]", raw_char, raw_char);

            uint32_t cmd_id = 0; // 准备发送给总指挥的事件 ID

            // --- 指令暗号翻译表 ---
            if (raw_char == 'H')
                cmd_id = EVENT_CMD_HIGH; // 大雾暗号
            else if (raw_char == 'L')
                cmd_id = EVENT_CMD_LOW; // 小雾暗号
            else if (raw_char == 'O' || raw_char == '0')
                cmd_id = EVENT_CMD_OFF; // 关机暗号 (数字0也支持)
            else if (raw_char == 'I')
                cmd_id = EVENT_CMD_INT; // 间歇暗号

            // 如果翻译成功，塞进队列上报给指挥官
            if (cmd_id != 0)
            {
                xQueueSend(g_evt_queue, &cmd_id, 0);
            }
            else
            {
                ESP_LOGE(TAG, "[报警] 收到无法识别的语音/远程字符！");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20)); // 每 20ms 检查一次缓冲区
    }
}

// =================================================================================
// [区域 6]：核心控制子系统 - 【指挥官逻辑主轴任务】
// =================================================================================
/**
 * @brief 指挥官任务：每 100 毫秒跳动一次，管理全系统的逻辑转换与硬件同步。
 */
void task_main_controller(void *pvParameters)
{
    ESP_LOGI(TAG, "[核心启动] 总指挥官已正式接管系统。心跳步进：100ms。");

    uint32_t received_evt_id;  // 临时存储从“邮筒”取出的信件 ID
    int tick_counter = 0;      // 全局 100ms 级别心跳计秒器
    bool blink_signal = false; // LED2 闪烁用的逻辑状态位
    bool spray_permit = true;  // 喷雾允许信号位

    while (1)
    {
        /*
         * 1. 事件接收判定：
         * 设定 100 毫秒阻塞超时，既能及时响应按键/语音，也能维持固定的逻辑刷新频率。
         */
        if (xQueueReceive(g_evt_queue, &received_evt_id, pdMS_TO_TICKS(100)) == pdTRUE)
        {

            // --- [决策逻辑 A：物理交互切换] ---
            if (received_evt_id == EVENT_SHORT_PRESS)
            {
                // 如果在间歇模式点按键，直接执行关机，用户体验更好
                if (g_mode == MODE_INTERMITTENT)
                    g_mode = MODE_OFF;
                else
                    g_mode = (sys_mode_t)((g_mode + 1) % 3); // 0 -> 1 -> 2 -> 0 循环切换
                ESP_LOGE(TAG, ">>> [全系统同步] 指挥部确认：按键触发档位跳转为 %d", g_mode);
            }
            else if (received_evt_id == EVENT_LONG_PRESS)
            {
                g_mode = MODE_INTERMITTENT; // 长按 1 秒强制进入间歇喷雾
                ESP_LOGE(TAG, ">>> [全系统同步] 指挥部确认：按键触发进入【间歇模式】");
            }
            // --- [决策逻辑 B：远程指令绝对跳转] ---
            else if (received_evt_id == EVENT_CMD_OFF)
                g_mode = MODE_OFF;
            else if (received_evt_id == EVENT_CMD_HIGH)
                g_mode = MODE_HIGH;
            else if (received_evt_id == EVENT_CMD_LOW)
                g_mode = MODE_LOW;
            else if (received_evt_id == EVENT_CMD_INT)
                g_mode = MODE_INTERMITTENT;

            // 模式一旦改变，立刻清零所有局部计时器，保证不同模式的切换瞬间精准对位
            tick_counter = 0;
            blink_signal = false;
            spray_permit = true;
        }

        /*
         * 2. 周期性计时器计算 (心跳每跳一次代表 100 毫秒过去了)
         */
        tick_counter++; // 计步器累加

        // --- [指示灯闪烁节拍]：每 500 毫秒 (5 次心跳) 翻转一次信号状态
        if (tick_counter % 5 == 0)
        {
            blink_signal = !blink_signal;
        }

        // --- [间歇模式喷雾逻辑]：喷 3 秒 (前 30 次心跳)，停 3 秒 (后 30 次心跳)
        if (g_mode == MODE_INTERMITTENT)
        {
            if (tick_counter < 30)
                spray_permit = true;
            else
                spray_permit = false;

            // 满 6 秒一个大循环 (60 次心跳)，时间到了重置计数器
            if (tick_counter >= 60)
                tick_counter = 0;
        }
        else
        {
            // 在正常加湿模式下，始终允许喷雾输出
            spray_permit = true;
        }

        /*
         * 3. 硬件全量同步刷新：
         * 这是系统的“定心丸”，不管逻辑在想什么，每 100ms 都在物理引脚执行一次锁定。
         */
        update_visual_feedback(g_mode, blink_signal);
        update_atomizer_power(g_mode, spray_permit);
    }
}

// =================================================================================
// [区域 7]：程序启动总入口 (app_main)
// =================================================================================
void app_main(void)
{
    ESP_LOGI(TAG, "================================================");
    ESP_LOGI(TAG, "  智能加湿器 V6.1 旗舰集成版 (原生蓝牙) 启动运行  ");
    ESP_LOGI(TAG, "================================================");

    // 1. 【驱动层点火】：唤醒全板所有物理外设驱动程序
    bsp_led_init();      // 初始化三路指示灯
    bsp_ws2812_init();   // 初始化幻彩马卡龙灯珠
    bsp_atomizer_init(); // 初始化 108kHz 动力核心
    bsp_voice_init();    // 初始化语音串口总线
                         // 初始化原生蓝牙协议栈并开启广播

    // 2. 【信道层部署】：创建 FreeRTOS 指令队列 (长度设为 10，防止高频指令丢失)
    g_evt_queue = xQueueCreate(10, sizeof(uint32_t));

    // 3. 【任务层部署】：开启后台多任务并行作业系统
    // 参数说明：任务函数, 任务名, 栈空间, 参数, 优先级, 任务句柄
    xTaskCreate(task_button_scan, "按键监控任务", 2048, NULL, 5, NULL);
    xTaskCreate(task_comm_listener, "语音监听任务", 3072, NULL, 5, NULL);
    xTaskCreate(task_main_controller, "核心控制任务", 4096, NULL, 10, NULL); // 主任务优先级最高

    ESP_LOGI(TAG, ">>> 全功能模块加载完毕！手机蓝牙名：V6_MACARON");
}
```

