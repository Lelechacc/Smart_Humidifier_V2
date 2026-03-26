# 💧 智能物联网加湿器 V10 (ESP32-C3 全栈硬核开发版)

<div align="center">
  <img src="https://img.shields.io/badge/MCU-ESP32--C3-blue?style=for-the-badge" />
  <img src="https://img.shields.io/badge/RTOS-FreeRTOS-lightgrey?style=for-the-badge" />
  <img src="https://img.shields.io/badge/EDA-JiaLiChuang-red?style=for-the-badge" />
  <img src="https://img.shields.io/badge/Voice-ASRPRO-8A2BE2?style=for-the-badge" />
</div>

---

## 🛠️ 项目概述与核心功能描述

本项目是一个基于 ESP32-C3 和 FreeRTOS 独立设计、打板并编写底层驱动的智能加湿器系统。V10 版本抛弃了早期的裸机大循环架构，实现了真正的多任务并发与软硬件解耦。

### 📌 实现了哪些具体功能？
1. **多模态雾化控制**：基于物理按键的长/短按，系统可在四个状态间精准流转：
   * **待机模式 (Off)**：整机低功耗，关闭 PWM 输出与灯效。
   * **持续大雾 (High)**：108kHz 满载谐振输出（限制在安全占空比内）。
   * **静音小雾 (Low)**：降低占空比，减少出雾量。
   * **间歇喷雾模式**：基于 RTOS 软件定时器，实现精准的“喷雾 3 秒，停止 3 秒”循环。
2. **离线语音控制**：接入天问 51 (ASRPRO) 语音识别模块。用户可通过语音指令直接跨越按键状态机，实现绝对指令跳转（如直接喊“打开大雾”）。
3. **状态同步全彩灯效**：使用 WS2812B 幻彩灯珠，不同雾化模式对应不同的马卡龙色系动态流光反馈。
4. **原生蓝牙 (BLE) 预留**：底层代码已初始化 BLE 协议栈，支持后续接入手机端进行参数 OTA 或动态雾量曲线调节。

---

## ⚙️ 硬件选型与系统架构 (BOM & Hardware)

硬件部分采用嘉立创 EDA 自行设计，双层 PCB 布局，高度集成：
* **主控芯片**：乐鑫 ESP32-C3FN4 (RISC-V 单核，内置 4MB Flash，支持 Wi-Fi & BLE 5.0)。
* **电源管理**：
  * 充电模块：采用 **TC4056A** 线性充电 IC，负责锂电池的安全充放。
  * 电池保护：采用 **XB5306A** 一体化保护芯片，防止锂电池过充、过放与短路。
* **雾化驱动**：使用 **AO3400** N沟道增强型 MOS 管，配合功率电感，驱动微孔雾化片。
* **模块化扩展**：PCB 背面物理预留了 **4 组多功能并联排母**，支持多传感器（如温湿度）级联。

---

## 💻 软件架构与攻坚技术点 (Software & Solutions)

本项目抛弃了 Arduino 封装，完全基于 **ESP-IDF** 原生框架开发，并在以下三个关键点上进行了底层攻坚：

### 1. 软件级动力安全限位 (解决 MOS 管烧毁痛点)
微孔雾化片需要精准的 **108kHz** 硬件 PWM 驱动。在早期版本中，占空比失控会导致 MOS 管和电感热击穿。
* **实现方案**：在 `bsp_atomizer.c` 中，利用 ESP32 的 LEDC 外设生成高频 PWM。在应用层与驱动层之间加入拦截逻辑，强制将最高占空比物理锁定在 **45% (寄存器数值 135)**。即使业务层发生 Bug 传入非法大数值，底层也能强制削峰，实现设备 0 发烫。

### 2. GPIO 矩阵路由解决走线冲突
天问 ASRPRO 模块的出厂固件强制要求 PA2 作为 TX 发送端。但这与加湿器主板 PCB 的最优物理走线产生了交叉冲突。
* **实现方案**：没有选择飞线或重新打板，而是深挖 ESP32-C3 的内部互联矩阵特性。在 `bsp_voice.c` 中，调用 `uart_set_pin(UART_NUM_1, 20, 21...)` API，在代码编译层将 **TX 和 RX 的物理引脚职责进行了互换**，完美适配了死区。

### 3. RMT 硬件接管纳秒级色彩时序
WS2812B 灯珠对数据线的高低电平时间要求达到纳秒级。如果用常规的 CPU 软件翻转 GPIO，在 FreeRTOS 多任务环境下极易被打断，导致灯珠闪烁、乱码。
* **实现方案**：在 `bsp_ws2812.c` 中，全面启用乐鑫专门用于红外和灯带控制的 **RMT (Remote Control) 硬件外设**。业务层只需将 RGB 数组丢进硬件缓存区，后续的纳秒级时序波形全部由 RMT 硬件自动发送，实现了 **0 CPU 占用率**。

---

## 📺 硬件实机图集 (Visuals)

<table align="center" style="border-collapse: collapse; border: none;">
  <tr>
    <td align="center" width="500" style="padding: 10px;">
      <div style="background: rgba(0,0,0,0.03); border-radius: 12px; border: 1px solid #eee; padding: 10px;">
        <b>实机演示 (幻彩灯与雾化联动)</b><br>
        <img src="images/实机演示_幻彩灯与雾化联动.jpg" width="460" style="border-radius: 8px;"><br>
      </div>
    </td>
    <td align="center" width="500" style="padding: 10px;">
      <div style="background: rgba(0,0,0,0.03); border-radius: 12px; border: 1px solid #eee; padding: 10px;">
        <b>硬件细节 (PCB正面静态图)</b><br>
        <img src="images/硬件细节_PCB正面静态图.jpg" width="460" style="border-radius: 8px;"><br>
      </div>
    </td>
  </tr>
  <tr>
    <td align="center" width="500" style="padding: 10px;">
      <div style="background: rgba(0,0,0,0.03); border-radius: 12px; border: 1px solid #eee; padding: 10px;">
        <b>硬件细节 (PCB背面与电池保护)</b><br>
        <img src="images/硬件细节_PCB背面与电池保护.jpg" width="460" style="border-radius: 8px;"><br>
      </div>
    </td>
    <td align="center" width="500" style="padding: 10px;">
      <div style="background: rgba(0,0,0,0.03); border-radius: 12px; border: 1px solid #eee; padding: 10px;">
        <b>系统全貌 (主板与全套外设)</b><br>
        <img src="images/系统全貌_主板与全套外设.jpg" width="460" style="border-radius: 8px;"><br>
      </div>
    </td>
  </tr>
</table>

---

## 📂 静态资源与打板资料下载 (一键获取)

以下文件托管于 GitHub Release，点击即可直接下载，方便二次开发与直接打样验证：

* 📦 **[一键打板：Gerber 生产打板文件 (.zip)](https://github.com/Lelechacc/Smart_Humidifier_V2/releases/download/v10.0/Gerber_Smart_Humidifier_V10.zip)** * 📎 **[图纸：电路原理图 PDF (自研 V10 版)](https://github.com/Lelechacc/Smart_Humidifier_V2/releases/download/v10.0/_.V10_SCH_Schematic1_2026-03-26.pdf)** * 🛠️ **[源文件：嘉立创 EDA 工程原文件 (.eprj)](https://github.com/Lelechacc/Smart_Humidifier_V2/releases/download/v10.0/Smart_Humidifier_V10.eprj)** * 📝 **[源码仓库 (含底层驱动与业务代码)](main/)** ---
> 💡 *项目代码与电路设计均已通过实机验证，具备完整的软硬件闭环能力。*
