<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Smart Humidifier V10 | 智能加湿器旗舰版</title>
    <style>
        :root {
            /* 现代苹果风配色方案 */
            --glass-bg: rgba(255, 255, 255, 0.25); /* 磨砂玻璃背景色 */
            --glass-border: rgba(255, 255, 255, 0.35); /* 玻璃边框色 */
            --accent-blue: #007aff; /* 苹果标志性蓝色 */
            --main-text: #1d1d1f; /* 主要文本颜色 */
            --sub-text: #666; /* 次要文本颜色 */
        }

        * { margin: 0; padding: 0; box-sizing: border-box; }

        body {
            /* 使用高清海浪沙滩背景图，确保视觉清凉感 */
            background: url('https://images.unsplash.com/photo-1507525428034-b723cf961d3e?auto=format&fit=crop&w=2000&q=80') no-repeat center center fixed;
            background-size: cover;
            min-height: 100vh;
            display: flex;
            justify-content: center;
            align-items: flex-start;
            color: var(--main-text);
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, "Helvetica Neue", Arial, sans-serif;
            padding: 50px 20px;
        }

        /* 磨砂玻璃主容器 */
        .glass-panel {
            background: var(--glass-bg);
            backdrop-filter: blur(25px) saturate(180%);
            -webkit-backdrop-filter: blur(25px) saturate(180%);
            border: 1px solid var(--glass-border);
            border-radius: 35px;
            box-shadow: 0 10px 40px rgba(0,0,0,0.1);
            width: 100%;
            max-width: 1200px;
            padding: 50px;
            display: flex;
            flex-direction: column;
        }

        /* 顶部导航与标题 */
        header { text-align: center; margin-bottom: 50px; }
        h1 { font-size: 2.8rem; font-weight: 700; margin-bottom: 10px; color: #fff; text-shadow: 0 2px 10px rgba(0,0,0,0.2); }
        .version-badge { background: var(--accent-blue); color: white; padding: 5px 15px; border-radius: 20px; font-size: 0.9rem; font-weight: 600; display: inline-block; }

        /* 核心内容区 */
        .content-main { display: flex; gap: 40px; margin-bottom: 40px; flex-wrap: wrap; }

        /* 左侧视频区域 */
        .showcase-video { flex: 2; min-width: 350px; background: rgba(0,0,0,0.1); border-radius: 25px; overflow: hidden; }
        .showcase-video video { width: 100%; display: block; border-radius: 20px; }

        /* 右侧技术说明 (也采用玻璃框) */
        .technical-specs { flex: 1; min-width: 300px; display: flex; flex-direction: column; gap: 20px; }
        
        .spec-card {
            background: rgba(255, 255, 255, 0.15);
            backdrop-filter: blur(10px);
            border: 1px solid var(--glass-border);
            padding: 25px;
            border-radius: 25px;
            transition: 0.3s ease;
        }
        .spec-card:hover { background: rgba(255, 255, 255, 0.25); transform: translateY(-3px); }
        .spec-card h3 { font-size: 1.2rem; font-weight: 600; margin-bottom: 10px; }
        .spec-card p { font-size: 0.95rem; line-height: 1.6; color: rgba(0,0,0,0.7); }

        /* 硬件画廊区域 */
        .hardware-gallery { margin-bottom: 40px; }
        .hardware-gallery h2 { font-size: 1.8rem; margin-bottom: 25px; border-bottom: 1px solid var(--glass-border); padding-bottom: 10px; font-weight: 600; }
        .gallery-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 30px; }
        
        /* 硬件图片容器 (完美的圆角框) */
        .gallery-item {
            background: rgba(255, 255, 255, 0.15);
            border-radius: 25px;
            border: 1px solid var(--glass-border);
            overflow: hidden;
            box-shadow: 0 4px 15px rgba(0,0,0,0.05);
            transition: 0.3s ease;
        }
        .gallery-item:hover { transform: translateY(-5px); box-shadow: 0 8px 30px rgba(0,0,0,0.1); }
        .gallery-item img { width: 100%; height: 250px; object-fit: cover; display: block; } /* 确保图片完美贴合 */
        .gallery-item p { padding: 15px; text-align: center; font-size: 0.9rem; color: rgba(0,0,0,0.7); font-style: italic; }

        /* 底部下载与版权 */
        footer { text-align: center; border-top: 1px solid var(--glass-border); padding-top: 30px; margin-top: auto; }
        .btn-center { display: flex; justify-content: center; gap: 15px; flex-wrap: wrap; margin-top: 20px; }
        .btn { padding: 12px 30px; border-radius: 25px; text-decoration: none; font-weight: 600; font-size: 1rem; transition: 0.3s; }
        .btn-primary { background: #fff; color: var(--accent-blue); box-shadow: 0 4px 10px rgba(0,0,0,0.1); }
        .btn-primary:hover { background: #f5f5f7; transform: translateY(-2px); }
        .btn-secondary { background: rgba(0,0,0,0.1); color: #fff; }
        .btn-secondary:hover { background: rgba(0,0,0,0.2); }
    </style>
</head>
<body>

    <div class="glass-panel">
        
        <header>
            <h1>Smart Humidifier V10</h1>
            <span class="version-badge">最终交付稳定版 (全栈自研)</span>
        </header>

        <div class="content-main">
            
            <div class="showcase-video">
                <video src="images/实物全功能演示视频.mp4" controls poster="images/实机演示_幻彩灯与雾化联动.jpg"></video>
            </div>

            <aside class="technical-specs">
                <div class="spec-card">
                    <h3>🛡️ 防御性功率锁定</h3>
                    <p>底层驱动强制定格 108kHz 硬件 PWM 频率，并绝对限制占空比低于 45% (135)，配合零电平启动逻辑，彻底杜绝硬件烧毁隐患。</p>
                </div>
                <div class="spec-card">
                    <h3>🔄 协议引脚翻转</h3>
                    <p>利用 ESP32-C3 的 UART 矩阵路由技术，实现了 GPIO 物理职责翻转 (TX/RX 对调)，兵不血刃适配第三方离线语音模块走线死区。</p>
                </div>
                <div class="spec-card">
                    <h3>⏱️ RMT 硬件时序驱动</h3>
                    <p>全面引入原生 RMT 硬件接管 WS2812B 色彩发送，告别 CPU 轮询翻转，确保 FreeRTOS 多任务调度下灯效丝滑不卡顿。</p>
                </div>
            </aside>

        </div>

        <section class="hardware-gallery">
            <h2>硬件展示与系统全貌</h2>
            <div class="gallery-grid">
                
                <div class="gallery-item">
                    <img src="images/3D渲染_正面整体架构.png" alt="3D正面整体架构">
                    <p>嘉立创 EDA 3D 渲染正面整体架构</p>
                </div>
                
                <div class="gallery-item">
                    <img src="images/硬件细节_PCB正面静态图.jpg" alt="硬件细节_PCB正面静态图">
                    <p>PCB 正面高度集成静态图</p>
                </div>

                <div class="gallery-item">
                    <img src="images/硬件细节_PCB背面与电池保护.jpg" alt="硬件细节_PCB背面与电池保护">
                    <p>PCB 背面：双重电池保护电路细节</p>
                </div>

                <div class="gallery-item">
                    <img src="images/系统全貌_主板与全套外设.jpg" alt="系统全貌_主板与全套外设">
                    <p>V10 最终版系统全套配件全貌</p>
                </div>

            </div>
        </section>

        <section class="hardware-gallery">
            <h2>开源资源与打板资料</h2>
            <div class="gallery-grid">
                
                <div class="gallery-item">
                    <img src="images/开发工具_天问51语音配置.png" alt="开发工具_天问51语音配置">
                    <p>离线语音配置工具链展示 (ASRPRO)</p>
                </div>
                
                <div class="gallery-item" style="background: rgba(0,122,255,0.05);">
                    <div style="padding: 50px 20px; text-align: center;">
                        <h3 style="color: var(--accent-blue);">📎 V10 Gerber (打板文件)</h3>
                        <p style="font-size: 0.9rem; margin-top: 10px;">已上传，广大网友可直接下载使用。</p>
                    </div>
                    <p>位于 Hardware/ 目录下的核心 Gerber 压缩包</p>
                </div>
            </div>
        </section>

        <footer>
            <div class="btn-center">
                <a href="https://github.com/Lelechacc/Smart_Humidifier_V2" class="btn btn-primary">查看 GitHub 源码</a>
                <a href="Hardware/电路原理图_智能加湿器V10.pdf" class="btn btn-secondary">下载原理图 PDF</a>
            </div>
            <p style="margin-top: 30px; font-size: 0.8rem; color: rgba(0,0,0,0.6);">© 2026 Developed by Lelechacc. |致力于嵌入式全栈协同开发 | V10 Final Release</p>
        </footer>

    </div>

</body>
</html>
