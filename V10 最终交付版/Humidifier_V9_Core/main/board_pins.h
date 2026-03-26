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