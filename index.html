# 💧 智能物联网加湿器 V10 (全栈自研旗舰版)

<div align="center">
  <img src="https://img.shields.io/badge/SoC-ESP32--C3-blue?style=for-the-badge" />
  <img src="https://img.shields.io/badge/OS-FreeRTOS-lightgrey?style=for-the-badge" />
  <img src="https://img.shields.io/badge/Hardware-JiaLiChuang-red?style=for-the-badge" />
  <img src="https://img.shields.io/badge/Workflow-AI%20Collaborated-8A2BE2?style=for-the-badge" />
</div>

---

## 📺 硬件风采与系统全貌 (Visual Showcase)

<table align="center">
  <tr>
    <td align="center" width="500">
      <b>📸 实机运行 (幻彩与雾化联动)</b><br>
      <img src="images/实机演示_幻彩灯与雾化联动.jpg" width="480"><br>
      <i>108kHz 驱动下稳定出雾，WS2812B 提供动态反馈</i>
    </td>
    <td align="center" width="500">
      <b>🔍 硬件细节 (PCB 背面与焊接)</b><br>
      <img src="images/硬件细节_PCB背面与电池保护.jpg" width="480"><br>
      <i>展示自研 PCB 背面走线与电池保护电路细节</i>
    </td>
  </tr>
  <tr>
    <td align="center" width="500">
      <b>🎨 3D 建模 (正面架构)</b><br>
      <img src="images/3D渲染_正面整体架构.png" width="480"><br>
      <i>高度集成的双层板设计，合理的元器件热力分布</i>
    </td>
    <td align="center" width="500">
      <b>📦 系统全景 (全套外设)</b><br>
      <img src="images/系统全貌_主板与全套外设.jpg" width="480"><br>
      <i>V10 最终版配套：主控板、电池、天问语音及雾化模组</i>
    </td>
  </tr>
</table>

---

## ⚔️ 核心技术攻坚 (Technical Deep Dive)

本项目的代码逻辑并非简单的功能堆砌，而是针对嵌入式底层难点进行的深度优化：

### 1. 🛡️ 硬件级动力安全限位 (Power Safety)
* **难点**：微孔雾化片谐振频率为 **108kHz**。若 PWM 功率过载，MOS 管极易击穿。
* **对策**：在 `bsp_atomizer.c` 中通过 AI 协同构建防御逻辑。无论上层指令如何变换，占空比均被物理锁定在 **45% (数值 135)** 以下，从根源保护 PCB 电感不发烫、不烧毁。

### 2. 🔄 串口引脚“乾坤大挪移” (UART Matrix)
* **难点**：对接“天问 ASRPRO”模块时，遭遇硬件引脚走线交叉冲突，常规方法需割线飞线。
* **对策**：利用 ESP32-C3 的 **GPIO 矩阵路由** 特性，在 `bsp_voice.c` 中调用 `uart_set_pin()` 将 **TX/RX 职责进行代码级翻转**，优雅解决硬件死区。

### 3. ⏱️ 纳秒级 RMT 信号发生器 (WS2812B)
* **难点**：传统 GPIO 翻转受 FreeRTOS 任务调度干扰，会导致幻彩灯珠闪烁或死机。
* **对策**：引入 **RMT (远程控制) 硬件外设**。将色彩数据直接填入硬件缓存，由底层硬件精准发送时序，实现 **0 CPU 占用** 的极速响应。

---

## 🧩 扩展性与未来演进 (Future Roadmap)

* **模块化接口**：PCB 背面专门预留了 **4 组并联排母接口**，支持后续多板级联与分布式传感器阵列接入。
* **软件架构**：基于 **FreeRTOS 事件队列**，已预留原生蓝牙 BLE 协议栈接口。

---

## 📂 核心资源导航

* 📝 **[源码包 (含详细中文注释)](main/)**
* 📎 **[电路原理图 PDF (自研 V10 版)](Hardware/电路原理图_智能加湿器V10.pdf)**
* 📦 **[Gerber 生产打板文件](Hardware/Gerber_Smart_Humidifier_V10.zip)**

---
© 2026 Developed by Lelechacc. | 拥抱 AI 协同开发新时代
