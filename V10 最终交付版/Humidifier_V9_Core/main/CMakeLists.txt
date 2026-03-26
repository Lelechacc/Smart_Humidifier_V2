# =================================================================================
# 【指挥部 main】构建配置文件 - V8 终极修复版
# =================================================================================

idf_component_register(
    SRCS 
        "main.c" 
        "bsp_led.c" 
        "bsp_ws2812.c" 
        "bsp_atomizer.c" 
        "bsp_voice.c"
    INCLUDE_DIRS "."
    
    # 【核心修正】：显式点名所有需要的驱动模块
    # 只要这里写全了，driver/gpio.h 就一定能找到
    REQUIRES 
        led_strip           # 幻彩灯组件
        esp_driver_ledc     # PWM (LEDC) 驱动
        esp_driver_gpio     # 【最关键：修复 driver/gpio.h 报错】
        driver              # 通用驱动基础
)