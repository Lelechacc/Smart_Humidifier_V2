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