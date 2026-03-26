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