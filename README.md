# 💧 智能物联网加湿器 V10 (全栈自研旗舰版)

<div align="center">
  <img src="https://img.shields.io/badge/SoC-ESP32--C3-blue?style=for-the-badge" />
  <img src="https://img.shields.io/badge/OS-FreeRTOS-lightgrey?style=for-the-badge" />
  <img src="https://img.shields.io/badge/Hardware-JiaLiChuang-red?style=for-the-badge" />
  <img src="https://img.shields.io/badge/Workflow-AI%20Collaborated-8A2BE2?style=for-the-badge" />
</div>

---

## 🚀 项目迭代：从失败到量产的蜕变 (V3 ➡️ V10)

这是一个充满了工程挑战与个人成长的项目。最初的 **V3 版本**（老师看了直摇头）存在严重的功能单一和硬件过热问题。经历了七个版本的技术攻坚，我从 V3 的功能堆砌，重构到了 V10 的基于 FreeRTOS 的事件驱动架构，不仅实现了 108kHz 高频谐振控制，更从根源上解决了 V3 版本的 MOS 管烧毁隐患，最终交付了一个具备 **“量产潜力”** 的旗舰级版本。

---

## 📺 硬件风采与全系统展示 (Visual Showcase)

<table align="center" style="border-collapse: collapse; border: none;">
  <tr>
    <td align="center" width="500" style="padding: 15px;">
      <div style="background: rgba(255, 255, 255, 0.1); border-radius: 20px; border: 1px solid rgba(255, 255, 255, 0.2); box-shadow: 0 4px 15px rgba(0,0,0,0.1); padding: 10px;">
        <b>📸 实物演示 (全功能联动)</b><br>
        <img src="images/实机演示_幻彩灯与雾化联动.jpg" alt="实机演示" width="460" style="border-radius: 10px;"><br>
        <i>V10最终版实机演示：完整展示按键、语音唤醒、原生蓝牙、多色幻彩与超声波雾化功能的协同运行。</i>
      </div>
    </td>
    <td align="center" width="500" style="padding: 15px;">
      <div style="background: rgba(255, 255, 255, 0.1); border-radius: 20px; border: 1px solid rgba(255, 255, 255, 0.2); box-shadow: 0 4px 15px rgba(0,0,0,0.1); padding: 10px;">
        <b>🔍 硬件细节 (PCB正面高度集成图)</b><br>
        <img src="images/硬件细节_PCB正面静态图.jpg" alt="PCB正面" width="460" style="border-radius: 10px;"><br>
        <i>展示主控ESP32-C3、电源管理TC4056A与雾化驱动MOS管AO3400的高度集成布局。</i>
      </div>
    </td>
  </tr>
  <tr>
    <td align="center" width="500" style="padding: 15px;">
      <div style="background: rgba(255, 255, 255, 0.1); border-radius: 20px; border: 1px solid rgba(255, 255, 255, 0.2); box-shadow: 0 4px 15px rgba(0,0,0,0.1); padding: 10px;">
        <b>🔍 硬件细节 (PCB背面电池保护细节)</b><br>
        <img src="images/硬件细节_PCB背面与电池保护.jpg" alt="背面细节" width="460" style="border-radius: 10px;"><br>
        <i>展示自研PCB背面走线、电池安装细节以及XB5306A一体化保护电路。</i>
      </div>
    </td>
    <td align="center" width="500" style="padding: 15px;">
      <div style="background: rgba(255, 255, 255, 0.1); border-radius: 20px; border: 1px solid rgba(255, 255, 255, 0.2); box-shadow: 0 4px 15px rgba(0,0,0,0.1); padding: 10px;">
        <b>🎨 3D 渲染图 (正面架构)</b><br>
        <img src="images/3D渲染_正面整体架构.png" alt="3D正面" width="460" style="border-radius: 10px;"><br>
        <i>高度集成的双层PCB设计，合理的元器件热力分布，展现嘉立创EDA画板闭环设计。</i>
      </div>
    </td>
  </tr>
</table>

---

## ⚔️ 核心技术攻坚 (Technical Deep Dive)

本项目的核心含金量在于针对嵌入式底层难点进行的防御性编程与性能调优：

### 1. 🛡️ 硬件级动力安全保护 (MOS FET 防烧毁)
* **情节与难点**：微孔雾化片谐振频率为 **108kHz**。 在 V3 版本中，由于 PWM 占空比失控，导致 MOS 管和功率电感瞬间高温烧毁。
* **对策与实现**：在底层代码 `Core/Drivers/bsp_atomizer.c` 中，利用 AI 辅助构建了防御性编程逻辑。通过高阶参数计算，强行将 PWM 占空比在物理层面削峰并锁定在 **45% (数值 135)** 以下，实现了 0 发烫运行。 **(Core/Drivers/bsp_atomizer.c)**

### 2. 🔄 串口引脚软件层对调 (ASRPRO 兵不血刃适配)
* **难点**：天问 ASRPRO 模块强制 PA2 为 TX，这导致与 V10 PCB 的物理走线产生了交叉冲突。割线飞线不仅麻烦，更影响信号完整性。
* **对策与实现**：利用 ESP32-C3 的 **GPIO 矩阵路由** 特性，在 `Core/Drivers/bsp_voice.c` 中巧妙调用 `uart_set_pin()` 将 **TX/RX 物理职责代码级对调**，优雅解决了走线冲突。 **(Core/Drivers/bsp_voice.c)**

### 3. ⏱️ 纳秒级色彩时序发生器 (RMT 0 CPU占有率)
* **难点**：WS2812B 幻彩灯珠对时序要求达到纳秒级。传统 GPIO 翻转会由于 FreeRTOS 的任务调度被打断，导致灯珠闪烁、主循环卡顿。
* **对策与实现**：引入乐鑫原生的 **RMT (Remote Control) 硬件外设**。将色彩数据直接填入硬件缓存，由底层硬件精准发送纳秒级时序波形，实现 **0 CPU占用** 的极速视觉响应。 **(Core/Drivers/bsp_ws2812.c)**

---

## 📂 核心资料导航与开源利器

* 📝 **[源码包 (含深度的中文架构注释)](Core/)**
* 📎 **[电路原理图 PDF (自研 V10 版)](Hardware/Humidifier_V10_Schematic_2026-03-26.pdf)**
* 🛠️ **[嘉立创 EDA 工程原文件 (EPRJ)](Hardware/Smart_Humidifier_V10.eprj)**
* 📦 **[Gerber 生产打板文件 (可直接扔给板厂打样)](Hardware/Gerber_V10_Smart_Humidifier.zip)**

---
© 2026 Developed by Lelechacc. | 拥抱全栈工程思维，致力于物联网系统重构与优化
