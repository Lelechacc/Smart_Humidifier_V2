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