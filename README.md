# ESP32-S3 圆形屏智能语音助手

## 项目概述

基于 ESP32-S3 的桌面级智能语音助手，集成 360x360 圆形 LCD 触摸屏、语音交互、WiFi/BLE 配网、OTA 升级、低功耗管理。四层软件架构，模块间通过发布/订阅事件总线解耦，支持同步与异步两种发布模式。

开发环境：ESP32-S3（ESP-IDF v6.0）、FreeRTOS、LVGL 9、SquareLine Studio。C。

## 项目职责

整体软件架构设计：自底向上 DRV -> BSP -> SVC -> APP。DRV 层封装 LCD、触摸、音频、SD 卡等外设驱动；SVC 层独立出 WiFi、HTTP、OTA、电源管理等模块，通过服务管理器管理生命周期。模块间通过事件总线（同步发布/异步入队）与函数调用（同步阻塞执行/异步投递执行）两种机制解耦；APP 层运行 LVGL 图形界面。

显示屏与 GUI：ST77916 360x360 圆形 LCD，QSPI 四线驱动，LVGL 9 渲染。三屏导航体系：Home（时间/日期显示）、Desktop（应用卡片入口）、Settings（亮度调节、音量调节、WiFi 配网面板、重置按键）。触摸 CST816S 驱动左右滑/下滑切换页面，Settings 屏按钮已绑定事件回调。

WiFi & BLE 配网：Soft-AP 强制门户（HTTP 服务器 + DNS 重定向）+ BLE 广播两种配网方式。配网时显示屏弹出 PIN 码与连接指引，凭据写入 NVS 持久化存储，支持 WPA3 与断线自动重连。开机无凭据自动进入配网模式。

OTA 远程升级：双分区 + 应用回滚。WiFi 连通后自动请求版本号，检测新固件后下载到备用分区，重启切换。启动失败 bootloader 自动回滚旧版本。

低功耗管理：四级电源状态逐级递降——Active 全速运行、Idle 背光降为 5%、Light Sleep CPU 暂停背光关闭、Deep Sleep 全系统断电。FreeRTOS 任务每秒检测空闲时长，15 秒降 Idle，1 分钟降 Light Sleep，2 小时降 Deep Sleep。触摸 GPIO 中断可逐级唤醒恢复。对外提供 API 供其他模块复位空闲计时器。

语音交互与媒体：ES8311 DAC + ES7210 ADC，I2S 48kHz/16bit 双向音频。支持 SD 卡音乐播放、录音保存与回放。音量 0-100%、增益 0-37dB 可调。