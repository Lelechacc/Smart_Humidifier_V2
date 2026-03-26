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