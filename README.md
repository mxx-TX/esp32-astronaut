# ESP-VoCat: 太空人智能语音助手

ESP-VoCat 是一个基于 ESP32-S3 的端侧智能语音助手参考设计。集成 1.32 寸圆形触摸彩屏、WiFi/BLE 连接和低功耗电源管理，适用于智能家居中控、桌面助手等场景。

---

## 特性

- **ESP32-S3** 双核 LX7 @240MHz，内置 NPU 加速器
- **1.32 寸 360x360 圆形 LCD** (ST77916 QSPI  40MHz)
- **电容触摸** (CST816S I2C)
- **双麦克风 + 扬声器** (ES7210 ADC + ES8311 DAC, I2S 48KHz)
- **WiFi 2.4GHz + BLE 5.0**
- **三级电源管理**: 活跃(150mA) -> 浅睡(3mA) -> 深睡(10uA)
- **端侧语音管线**: 唤醒词检测 + ASR + LLM + TTS

---

## 硬件规格


| 项目    | 参数                                     |
| ----- | -------------------------------------- |
| SoC   | ESP32-S3, Xtensa LX7 dual-core @240MHz |
| PSRAM | 8MB Octal SPI PSRAM (OPI)              |
| Flash | 16MB Quad SPI                          |
| 屏幕    | 1.32 寸圆形, 360x360, ST77916 QSPI        |
| 触摸    | CST816S, I2C 100KHz                    |
| 音频输入  | 2x PDM Mic -> ES7210 ADC               |
| 音频输出  | NS4158 功放 -> ES8311 DAC                |
| 无线    | 2.4GHz WiFi + BLE 5.0, PCB 天线          |
| 指示灯   | GPIO43 状态 LED                          |
| 存储    | MicroSD (SDMMC 1-bit)                  |
| 供电    | USB-C 5V, 内置锂电充电                       |
| 功耗    | 活跃 150mA, 空闲 80mA, 浅睡 3mA, 深睡 10uA     |




### 引脚分配


| 功能       | 引脚                                                                    | 说明                 |
| -------- | --------------------------------------------------------------------- | ------------------ |
| LCD QSPI | CS=14, DC=45, RST=3, BL=44, MOSI=46, MISO=13, SCLK=18, QWP=11, QHD=12 | ST77916 @40MHz     |
| LCD 电源使能 | GPIO9                                                                 | 高电平开启              |
| 触摸 I2C   | SDA=2, SCL=1, INT=10                                                  | CST816S, 地址 0x15   |
| 音频 I2S   | MCLK=42, BCLK=40, WS=39, DOUT=41, DIN=15                              | 48KHz 采样           |
| 音频 I2C   | 同触摸总线                                                                 | DAC 0x18, ADC 0x40 |
| 功放使能     | GPIO4                                                                 | PA_EN              |
| 状态 LED   | GPIO43                                                                | 高电平点亮              |
| SD 卡     | CLK=16, CMD=38, D0=17                                                 | SDMMC 1-bit        |


---



## 软件架构



### 四层设计

- **main**: 入口、启动编排
- **app**: 应用状态机、LVGL GUI、语音管线、SquareLine Studio UI
- **svc**: 服务管理器、事件总线、电源管理、WiFi、显示、音频等
- **bsp / drv**: 板级封装、芯片驱动 (ST77916 / CST816S / ES8311 / ES7210)



### 电源管理

三级降级状态机。触摸检测通过 GPIO10 电平轮询 (10ms 间隔)，不进 GPIO 中断，不与 LVGL I2C 触摸读取冲突。

1. **PM_ACTIVE**: 背光 100%, WiFi 在线, ~150mA
2. **PM_IDLE**: 背光 5%, CPU 降频, ~80mA (15s 无操作进入)
3. **PM_LIGHT_SLEEP**: 背光关, CPU 暂停, ~3mA (60s 无操作进入)
4. **PM_DEEP_SLEEP**: CPU 掉电, ~10uA (30min 无操作或 API 调用进入)

深睡唤醒流程: 触摸 GPIO10 -> CPU 冷启动 -> app_main() -> RTC 数据校验 -> IDLE 就绪。深睡数据通过 RTC_SLOW_MEM 保存，包含 magic 校验、唤醒次数和 CRC32 校验和。

---



## 交互方式

### WiFi 配网

首次开机或无保存凭据时自动进入配网:

1. 设备开启 SoftAP (SSID: Astronaut_Setup)
2. 设备开启 BLE 广播
3. 手机连接 AP 或扫描 BLE，浏览器弹出配网页
4. 输入 WiFi 密码 + PIN 码



### 触摸交互

- 轻触: 唤醒设备或确认
- 滑动: 切换功能页面
- 长按 (预留): 进入配网模式



### 语音交互

1. 唤醒词触发 (如 [小星小星])
2. LED 亮起，进入聆听状态
3. ASR 将语音转为文字
4. LLM 生成回复
5. TTS 播报回复

---



## 快速开始



### 环境


| 工具                | 版本                          |
| ----------------- | --------------------------- |
| ESP-IDF           | 6.0.2                       |
| LVGL              | 9.3                         |
| SquareLine Studio | 1.4+                        |
| 编译器               | xtensa-esp-elf (esp-15.2.0) |




### 构建

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash monitor
```



