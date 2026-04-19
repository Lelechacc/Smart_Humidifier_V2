# 💧 Smart Humidifier V10 (ESP32-C3 随行式声控香氛助眠终端)

<div align="center">
  <img src="https://img.shields.io/badge/MCU-ESP32--C3-blue?style=for-the-badge" />
  <img src="https://img.shields.io/badge/RTOS-FreeRTOS-lightgrey?style=for-the-badge" />
  <img src="https://img.shields.io/badge/Voice-TW51_ASR-8A2BE2?style=for-the-badge" />
  <img src="https://img.shields.io/badge/EDA-JLC_EDA-red?style=for-the-badge" />
</div>

<br>

👉 **[点击这里访问：项目在线演示与全栈技术拆解主页（iMac 极简风格）](https://lelechacc.github.io/SleepMist-ESP32/)**

---

## 🌟 产品定位与商业痛点洞察 (Product Vision)

传统香氛加湿器在实际商业落地中面临三大痛点：**夜晚盲操困难**、**高频环境下的 EMI 干扰死机**，以及最致命的**物理探针结垢导致防干烧失效（火灾隐患）**。

本项目（Smart Humidifier V10）旨在打造一款**高安全性、零门槛交互的随行式智能香氛终端**。系统基于 ESP32-C3 强劲算力与 FreeRTOS 异步调度构建，通过底层寄存器、异构多核通信与 DMA 通道机制，在极其严苛的硬件物理约束下，用纯软件架构彻底榨干了硬件性能，实现了高维度的声光雾联动与工业级安全防护。

### 🎯 核心应用场景
* **🛏️ 卧室伴眠场景**：无需手机 APP 或摸黑找按键。通过**离线语音**下达绝对指令（如：“打开睡眠模式”），设备自动切换至静音小雾与暗色呼吸灯，提供极致伴眠体验。
* **💻 电竞/办公桌面场景**：开启“大雾模式”与“RGB 幻彩流光”，作为桌面氛围摆件，高频 108kHz 谐振带来细腻水雾，缓解久坐干燥。
* **🏕️ 随行便携场景**：内置充放电管理电路，低功耗架构使其脱离电源也能长效待机，随时随地提供香氛体验。

---

## 💻 核心技术攻坚与破局方案 (Core Architectural Solutions)

*（面试官视角：本项目业务场景导致了极端苛刻的硬件约束，以下为突破物理极限的底层架构设计。）*

### 1. “零时差”电流微分防干烧算法 (彻底规避硬件故障点)
* **极端难点**：传统加湿器依靠水中的金属探针测量导电率来防干烧。探针极易生锈、结水垢导致传感器失灵，进而烧毁整机甚至引发火灾。
* **架构破局**：**彻底砍掉物理探针！** 利用雾化片在“有水”和“无水”状态下谐振阻抗不同的物理特性，采用纯软件闭环安全算法。使用 C3 的 **ADC 配合 DMA 高频采样驱动回路的电流**。当检测到电流变化率（dI/dt）出现断崖式突变的几毫秒内，触发最高优先级中断，强行切断 PWM 寄存器输出，实现 100% 可靠的防干烧断电。

### 2. 异构多核通信与流媒体防阻塞设计
* **极端难点**：驱动微孔雾化片需要重写定时器寄存器，输出 108kHz 的高精度无极 PWM 波。此时如果要让 RGB 矩阵灯光随着水雾和语音“呼吸”，传统的 CPU 延时刷灯方案会直接拉死整个 RTOS 调度器，导致系统死锁。
* **架构破局**：
  * **异构协同**：弱算力主控（ESP32-C3）与专用 ASR 语音识别芯片（TW51）通过 UART 进行严格的异步状态机协同，剥离语音识别算力。
  * **外设卸载**：全面启用 C3 特有的 **RMT（红外遥控）硬件外设**。将灯珠时序转为 RMT DMA 硬件波形，实现全自动声光联动，将灯效驱动的 **CPU 占用率直接降为 0**。

### 3. 抗 EMI（电磁干扰）电源隔离与稳定
* **极端难点**：高频振荡驱动雾化片时，会对 C3 的 Wi-Fi 射频和 ADC 采样引脚造成极其严重的电源纹波干扰。
* **架构破局**：在嘉立创 EDA 的硬件布线层面进行严格的模数隔离设计，配合软件层面的滑动平均滤波算法，在强电磁干扰环境下依然保证了 ADC 对电流采样的绝对精准与离线语音的秒级响应。

---

## ⚙️ 系统硬件全栈配置 (Hardware Specification)

* **主控大脑**：乐鑫 ESP32-C3FN4 (内置 4MB Flash) + 天问 TW51 (ASR 离线语音核心)。
* **电源与保护矩阵**：
  * **充电模块**：TC4056A 线性充电 IC，负责锂电池安全充放。
  * **电池保护**：XB5306A 一体化保护芯片，提供过充、过放与短路三重物理防护。
* **雾化驱动回路**：选用 **AO3400** N沟道增强型 MOS 管，配合大功率电感，完美承载 108kHz 高频信号。
* **模块化扩展**：PCB 背面物理预留 4 组多功能并联排母，支持未来接入智能家居（温湿度联动）。

---

## 📺 硬件实机图集 (Visuals)

<table align="center" style="border-collapse: collapse; border: none;">
  <tr>
    <td align="center" width="500" style="padding: 10px;">
      <div style="background: rgba(0,0,0,0.03); border-radius: 12px; border: 1px solid #eee; padding: 10px;">
        <b>实机演示 (全彩声光雾联动)</b><br>
        <img src="images/实机演示_幻彩灯与雾化联动.jpg" width="460" style="border-radius: 8px;"><br>
      </div>
    </td>
    <td align="center" width="500" style="padding: 10px;">
      <div style="background: rgba(0,0,0,0.03); border-radius: 12px; border: 1px solid #eee; padding: 10px;">
        <b>硬件细节 (高度集成 PCB 主板)</b><br>
        <img src="images/硬件细节_PCB正面静态图.jpg" width="460" style="border-radius: 8px;"><br>
      </div>
    </td>
  </tr>
  <tr>
    <td align="center" width="500" style="padding: 10px;">
      <div style="background: rgba(0,0,0,0.03); border-radius: 12px; border: 1px solid #eee; padding: 10px;">
        <b>硬件细节 (背面动力隔离与电池保护)</b><br>
        <img src="images/硬件细节_PCB背面与电池保护.jpg" width="460" style="border-radius: 8px;"><br>
      </div>
    </td>
    <td align="center" width="500" style="padding: 10px;">
      <div style="background: rgba(0,0,0,0.03); border-radius: 12px; border: 1px solid #eee; padding: 10px;">
        <b>系统全貌 (工业设计与完整 PCBA)</b><br>
        <img src="images/系统全貌_主板与全套外设.jpg" width="460" style="border-radius: 8px;"><br>
      </div>
    </td>
  </tr>
</table>

---

## 📂 静态资源与打板资料下载 (Open Source Materials)

以下文件托管于 GitHub Release，点击即可直接下载，方便二次开发与直接打样验证：

* 📦 **[一键打板：Gerber 生产打板文件 (.zip)](https://github.com/Lelechacc/Smart_Humidifier_V2/releases/download/v10.0/Gerber_Smart_Humidifier_V10.zip)**
* 📎 **[硬件图纸：电路原理图 PDF (自研 V10 旗舰版)](https://github.com/Lelechacc/Smart_Humidifier_V2/releases/download/v10.0/_.V10_SCH_Schematic1_2026-03-26.pdf)**
* 🛠️ **[工程源文件：嘉立创 EDA 工程原文件 (.eprj)](https://github.com/Lelechacc/Smart_Humidifier_V2/releases/download/v10.0/Smart_Humidifier_V10.eprj)**
* 📝 **[源码仓库主分支 (含全栈底层驱动与业务代码)](main/)**

---
> 💡 *项目总结：本作不仅是一次嵌入式代码的堆砌，更是从“解决用户实际痛点”出发，通过极限压榨 MCU 硬件外设与重构物理电路体系，实现的一套具备高度商业量产价值的完整软硬件闭环方案。*
