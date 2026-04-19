# 💧 Smart Humidifier V10 (ESP32-C3 全栈旗舰级智能香氛终端)

<div align="center">
  <img src="https://img.shields.io/badge/MCU-ESP32--C3-blue?style=for-the-badge" />
  <img src="https://img.shields.io/badge/RTOS-FreeRTOS-lightgrey?style=for-the-badge" />
  <img src="https://img.shields.io/badge/EDA-JLC_EDA-red?style=for-the-badge" />
  <img src="https://img.shields.io/badge/Voice-ASRPRO-8A2BE2?style=for-the-badge" />
</div>

---

## 🌟 产品定位与设计初衷 (Product Vision)

传统小型加湿器往往功能单一、交互简陋（仅靠单按键盲操），且在夜晚使用时存在痛点。
本项目（Smart Humidifier V10）旨在打造一款**高颜值、零门槛交互、高安全性的桌面级智能香氛加湿终端**。系统基于 ESP32-C3 强劲算力与 FreeRTOS 异步调度构建，不仅实现了多模态雾化与马卡龙动态光效的声光联动，更通过硬件级设计彻底解决了设备发热、烧毁的安全隐患。

### 🎯 核心应用场景 (Use Cases)
* **🛏️ 卧室伴眠场景**：无需手机 APP，无需夜晚摸黑找按键。通过**离线语音**（如：“打开睡眠模式”），设备自动切换至静音小雾，配合微弱的暖色呼吸灯，提供极致的伴眠体验。
* **💻 电竞/办公桌面场景**：开启“大雾模式”与“RGB 幻彩流光”，作为桌面氛围摆件，高频 108kHz 谐振带来细腻水雾，缓解久坐干燥。
* **🏠 全屋智能前哨节点**：得益于 ESP32-C3 强大的网络吞吐能力，设备底层已就绪 BLE 5.0 协议栈，不仅是一个加湿器，更是未来接入智能家居（智能温湿度联动）的拓展节点。

---

## ✨ 核心产品功能体验 (Core Features)

1. **四模态柔性雾化控制**：
   * **待机休眠 (Off)**：整机进入微安级超低功耗待机。
   * **持续大雾 (High)**：108kHz 满载谐振输出，迅速提升局部湿度。
   * **静音小雾 (Low)**：降低占空比，适合长时间无感润物。
   * **智能间歇模式**：基于 RTOS 软件定时器，实现精准的“喷雾 3 秒，休眠 3 秒”动态循环。
2. **“君子动口不动手”的离线语音**：深度集成 ASRPRO 语音核心，用户可跨越传统物理按键的状态机，通过语音口令下达绝对指令（如直接唤醒并“打开大雾”），毫秒级响应。
3. **状态同步全彩灯效**：采用 WS2812B 幻彩灯珠，不同雾化模式自动映射专属的马卡龙色系动态流光（如：大雾对应幻彩循环，小雾对应冰蓝呼吸）。

---

## ⚙️ 硬件选型与系统架构 (Hardware BOM)

硬件底板采用嘉立创 EDA 自行绘制，双层 PCB 紧凑布局，高度集成：
* **主控大脑**：乐鑫 ESP32-C3FN4 (RISC-V 架构，内置 4MB Flash，支持 Wi-Fi & BLE 5.0)。
* **电源与保护矩阵**：
  * **充电模块**：采用 **TC4056A** 线性充电 IC，负责锂电池安全充放。
  * **电池保护**：采用 **XB5306A** 一体化保护芯片，提供过充、过放与短路三重物理防护。
* **雾化驱动**：选用 **AO3400** N沟道增强型 MOS 管，配合功率电感，完美承载微孔雾化片的底层高频信号。
* **模块化扩展**：PCB 背面物理预留了 **4 组多功能并联排母**，支持多协议传感器（如 I2C 温湿度模块）的无缝级联。

---

## 💻 底层攻坚与技术痛点解决 (Technical Solutions)

*（本项目完全基于 **ESP-IDF** 原生框架开发，并在以下三个关键底层逻辑上进行了技术攻坚，体现了从应用层到驱动层的全栈把控能力。）*

### 1. 软件级动力安全限位 (解决 MOS 管烧毁痛点)
微孔雾化片依赖精准的 **108kHz** 硬件 PWM 驱动。早期版本中，占空比失控极易导致 MOS 管和电感热击穿。
* **硬核方案**：在 `bsp_atomizer.c` 中利用 LEDC 外设生成高频 PWM。在应用层与驱动层间构筑拦截逻辑，**强制将最高占空比物理锁定在 45% (寄存器数值 135)**。即使业务层传入非法满载数值，底层也能强制削峰，实现设备连续满载运行 0 发烫。

### 2. GPIO 矩阵路由解决 PCB 走线冲突
天问 ASRPRO 模块出厂固件强制要求 PA2 作为 TX 发送端，但这与加湿器主板 PCB 的最优物理走线产生严重交叉。
* **硬核方案**：摒弃飞线或重新打板的妥协方案，深挖 ESP32-C3 内部互联矩阵特性。在 `bsp_voice.c` 中调用 `uart_set_pin()` API，**在代码编译层将 TX 和 RX 的物理引脚职责进行动态映射互换**，完美化解硬件物理死区。

### 3. RMT 硬件接管纳秒级色彩时序
WS2812B 灯珠对数据线高低电平时间要求达到严苛的纳秒级。常规软件翻转 GPIO 在 FreeRTOS 多任务环境下极易被打断，导致灯珠闪烁、乱码。
* **硬核方案**：在 `bsp_ws2812.c` 中全面启用乐鑫专属的 **RMT (Remote Control) 硬件外设**。业务层仅需将 RGB 数组推入硬件缓存区，后续纳秒级时序波形全部由 RMT DMA 自动发送，实现灯效驱动 **0 CPU 占用率**。

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

## 📂 静态资源与打板资料下载 (Open Source Materials)

以下文件托管于 GitHub Release，点击即可直接下载，方便二次开发与直接打样验证：

* 📦 **[一键打板：Gerber 生产打板文件 (.zip)](https://github.com/Lelechacc/Smart_Humidifier_V2/releases/download/v10.0/Gerber_Smart_Humidifier_V10.zip)**
* 📎 **[硬件图纸：电路原理图 PDF (自研 V10 版)](https://github.com/Lelechacc/Smart_Humidifier_V2/releases/download/v10.0/_.V10_SCH_Schematic1_2026-03-26.pdf)**
* 🛠️ **[工程源文件：嘉立创 EDA 工程原文件 (.eprj)](https://github.com/Lelechacc/Smart_Humidifier_V2/releases/download/v10.0/Smart_Humidifier_V10.eprj)**
* 📝 **[源码仓库主分支 (含底层驱动与业务代码)](main/)**

---
> 💡 *本项目所有代码逻辑与电路设计均已通过实机多轮压测，具备完整的物联网智能硬件软硬闭环能力。*
