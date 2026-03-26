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