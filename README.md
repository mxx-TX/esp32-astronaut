# ESP32-S3 Astronaut 程序框架说明

## 一、目录层次（4 层架构）

`
D:\esp32item\esp32s3-astronaut\
├── main/                 第0层：入口
├── app/                  第1层：应用层
├── svc/                  第2层：服务层
│   └── framework/        框架子系统
├── bsp/                  第3层：板级支持包
├── drv/                  第3层：驱动层
├── os/                   操作系统抽象层
└── managed_components/   ESP Registry 组件
`

### 依赖方向

`
main → app → svc → bsp → drv
`

- main 依赖所有组件
- app 依赖 svc
- svc 依赖 bsp
- bsp 依赖 drv
- drv 无跨组件依赖

---

## 二、各层详细说明

### 第0层：main（入口）

| 文件 | 职责 |
|------|------|
| main/main.c | app_main()：NVS 初始化 → BSP 初始化 → 注册/启动 service → GUI 启动 → 应用状态机启动 |

**关键函数**：app_main(void)

**启动顺序**：
1. nvs_flash_init() —— NVS 存储
2. bsp_board_init(&h) —— 初始化所有硬件外设，返回 bsp_handles_t
3. svc_manager_register() —— 按依赖顺序注册 9 个 service
4. svc_manager_init_all(&h) —— 初始化所有 service（遍历 on_init）
5. gui_wifi_init() + app_gui_start() —— 启动 LVGL GUI
6. svc_manager_start_all() —— 启动所有 service（遍历 on_start）
7. app_fsm_init() + app_pipeline_init() + app_fsm_post_event(BOOT_DONE)

---

### 第1层：app（应用层）

| 文件 | 核心结构/函数 | 说明 |
|------|--------------|------|
| app/app_events.h | typedef enum { APP_EVT_BOOT_DONE, ... } app_event_t | 应用事件定义 |
| app/app_fsm.c/h | app_state_t 枚举（BOOT/IDLE/LISTENING/THINKING/SPEAKING）<br>app_fsm_init()、app_fsm_post_event() | 应用状态机，事件驱动 |
| app/app_gui.c/h | app_gui_start()、app_gui_update_state() | LVGL GUI 生命周期管理 |
| app/app_pipeline.c/h | app_pipeline_init(i2s_tx, i2s_rx) | 音频流水线初始化 |
| app/gui/gui_events.c/h | GUI 事件处理 | 屏幕触摸/按钮事件 |
| app/gui/gui_wifi.c/h | gui_wifi_init() | WiFi 配网相关的 GUI 界面 |
| app/gui/squareline_ui/ | SquareLine Studio 生成的 UI 代码 | 屏幕/组件/字体/图片 |

**状态机**：

`
BOOT ──BOOT_DONE──→ IDLE ──WAKE_WORD/TOUCH──→ LISTENING
                       ↑                          │
                       │                    ASR_DONE
                       │                          ↓
                       │                      THINKING
                       │                          │
                       │                    LLM_DONE
                       │                          ↓
                       │                      SPEAKING
                       │                          │
                       └──── TTS_DONE/TIMEOUT ←───┘
`

---

### 第2层：svc（服务层）

#### 2.1 framework（框架子系统）

##### svc_manager — 服务管理器

**核心结构**（svc/framework/svc_manager.h）：

`c
typedef struct svc_base {
    const char *name;
    const char **deps;
    int dep_count;
    svc_state_t state;
    esp_err_t (*on_init)(struct svc_base *, void *ctx);
    esp_err_t (*on_start)(struct svc_base *);
    esp_err_t (*on_stop)(struct svc_base *);
    esp_err_t (*on_deinit)(struct svc_base *);
} svc_base_t;
`

**公开接口**：

| 函数 | 说明 |
|------|------|
| svc_manager_register(svc_base_t *svc) | 注册一个服务到管理器 |
| svc_manager_init_all(void *ctx) | 遍历所有已注册服务，调用 on_init。ctx 传入 bsp_handles_t * |
| svc_manager_start_all() | 遍历所有已注册服务，调用 on_start |
| svc_manager_stop_all() | 遍历所有已注册服务，调用 on_stop |
| svc_manager_deinit_all() | 遍历所有已注册服务，调用 on_deinit |
| svc_manager_find(const char *name) | 按名称查找服务 |

##### svc_event — 事件总线

**事件 ID**（svc/framework/svc_event.h）：

`c
typedef enum {
    EVT_WIFI_CONNECTED,        // WiFi 连接成功
    EVT_WIFI_DISCONNECTED,     // WiFi 断开
    EVT_NETWORK_UP,            // 网络可用
    EVT_NETWORK_DOWN,          // 网络不可用
    EVT_WIFI_PROV_STARTED,     // 配网模式开始
    EVT_WIFI_CONNECTING,       // 正在连接 WiFi
    EVT_PM_IDLE,               // PM 进入空闲态
    EVT_PM_SLEEP,              // PM 进入浅睡
    EVT_PM_WAKE,               // PM 恢复活跃
    EVT_PM_DEEP_SLEEP,         // PM 进入深睡（给各服务清理用）
    EVT_USER = 100,            // 用户自定义事件的起始值
} svc_event_id_t;
`

**公开接口**：

| 函数 | 说明 |
|------|------|
| svc_event_subscribe(id, handler, ctx) | 订阅事件。handler 在发布者的任务上下文中同步执行 |
| svc_event_publish(id, data) | 同步发布事件，遍历所有订阅者 |
| svc_event_publish_async(id, data) | 异步发布（通过队列转发到 svc_evt 任务） |
| svc_event_unsubscribe(id, handler) | 取消订阅 |

#### 2.2 服务列表

| 服务 | 文件 | 注册顺序 | 说明 |
|------|------|---------|------|
| storage | svc_storage.c/h | 1 | NVS 键值存储封装 |
| indicator | svc_indicator.c/h | 2 | LED 指示灯控制（OFF/ON/SLOW/FAST/BREATHE） |
| wifi | svc_wifi.c/h | 3 | WiFi STA + SoftAP 配网 + HTTP 强制门户 + BLE 配网 |
| http | svc_http.c/h | 4 | HTTP 客户端 |
| time | svc_time.c/h | 5 | SNTP 时间同步 |
| display | svc_display.c/h | 6 | LVGL 显示 + 触摸初始化 |
| pm | svc_pm.c/h | 7 | 电源管理 —— 三级降级 + 深睡 |
| ota | svc_ota.c/h | 8 | OTA 升级 |
| audio | svc_audio.c/h | 9 | I2S 音频播放/录制/音量控制 |

各服务在 main.c 中按此顺序注册和初始化。

---

### 电源管理（svc_pm）

**头文件**（svc/svc_pm.h）：

`c
void svc_pm_activity(void);               // 通知系统有用户活动
void svc_pm_request_deep_sleep(void);     // 请求立即进入深度睡眠
extern svc_base_t g_svc_pm;               // 服务注册体
`

**状态机**（svc/svc_pm.c）：

`
PM_ACTIVE ──15s──→ PM_IDLE ──60s──→ PM_LIGHT_SLEEP
                                         │
                                  ext0触摸唤醒         30min定时器唤醒（无触摸）
                                      │                       │
                                      ▼                       ▼
                                  PM_ACTIVE              PM_DEEP_SLEEP
                                                              │
                                                    发布 EVT_PM_DEEP_SLEEP
                                                    → WiFi停止/背光0/LED灭
                                                    写 RTC 数据
                                                    ext0_wakeup(GPIO10)
                                                    esp_deep_sleep_start()
                                                              │
                                                    CPU掉电（~5uA）
                                                              │
                                                    触摸 GPIO10 ↓
                                                              ↓
                                                    ROM → app_main()
                                                    esp_sleep_get_wakeup_causes()
                                                    RTC magic+CRC校验
                                                    → 冷启动恢复 → IDLE
`

**超时配置**（毫秒）：
- MS_ACTIVE = 15000：活跃→空闲
- MS_IDLE = 60000：空闲→浅睡
- MS_DEEP = 30 * 60 * 1000：浅睡→深睡

**触摸检测**：通过 gpio_get_level(BSP_TOUCH_INT_GPIO) == 0 轮询（每 10ms 一次），检测触摸 IC（CST816S）的 INT 引脚电平。

**浅睡唤醒**：esp_sleep_enable_ext0_wakeup(BSP_TOUCH_INT_GPIO, 0) —— RTC GPIO 唤醒，不受数字 GPIO 中断影响。

**RTC 数据**（svc/svc_pm_rtc.h）：

`c
typedef struct {
    uint32_t magic;          // PM_RTC_MAGIC (0x504D444C)
    uint32_t boot_mode;      // 0=cold boot, 1=deep sleep wake
    uint32_t wakeup_count;   // 深睡唤醒次数
    uint32_t crc32;          // 校验和
} pm_rtc_data_t;

extern RTC_DATA_ATTR pm_rtc_data_t g_pm_rtc;
`

**深睡事件订阅者**：

| 服务 | 清理动作 |
|------|---------|
| svc_wifi | esp_wifi_stop() |
| svc_display | bsp_lcd_set_backlight(0) |
| svc_audio | 预留 |
| svc_indicator | s_mode = IND_OFF; bsp_led_off() |

---

### 第3层：bsp（板级支持包）

**核心结构**（bsp/bsp_board.h）：

`c
typedef struct {
    esp_lcd_panel_io_handle_t lcd_io;     // LCD IO 句柄（QSPI）
    esp_lcd_panel_handle_t lcd_panel;     // LCD 面板句柄
    i2c_master_bus_handle_t i2c_bus;      // I2C 总线句柄
    esp_lcd_touch_handle_t i2c_touch;     // 触摸句柄
    i2c_master_dev_handle_t i2c_dac;      // 音频 DAC 句柄（ES8311）
    i2c_master_dev_handle_t i2c_adc;      // 音频 ADC 句柄（ES7210）
    i2s_chan_handle_t i2s_tx;             // I2S TX 句柄
    i2s_chan_handle_t i2s_rx;             // I2S RX 句柄
} bsp_handles_t;
`

| 函数 | 文件 | 说明 |
|------|------|------|
| bsp_board_init(bsp_handles_t *h) | bsp_board.c | 初始化所有外设，填充 handles |
| bsp_board_lcd_power_on() | bsp_board.c | 打开 LCD 电源 |
| bsp_board_audio_power_on() | bsp_board.c | 打开音频功放电源 |
| bsp_lcd_init(io, panel) | bsp_lcd.c | 初始化 LCD |
| bsp_lcd_set_backlight(pct) | bsp_lcd.c | 设置背光百分比（0~100） |
| bsp_lcd_get_resolution(w, h) | bsp_lcd.c | 获取分辨率 |
| bsp_touch_init(i2c_bus, tp) | bsp_touch.c | 初始化触摸（CST816S） |
| bsp_touch_read(tp, data) | bsp_touch.c | 读取触摸数据 |
| bsp_audio_init(i2c_bus, ...) | bsp_audio.c | 初始化音频编解码器 |
| bsp_led_init() | bsp_led.c | 初始化 LED |
| bsp_sdcard_init() | bsp_sdcard.c | 初始化 SD 卡 |

**引脚定义**（bsp/bsp_board_cfg.h）：

| 模块 | 引脚 | 说明 |
|------|------|------|
| LCD SPI | MOSI=46, MISO=13, SCLK=18, CS=14, DC=45, RST=3, BL=44, QWP=11, QHD=12 | ST77916 QSPI |
| LCD 电源使能 | GPIO9 | LCD_EN |
| 触摸 I2C | SDA=2, SCL=1, INT=10 | CST816S，I2C 地址 0x15 |
| 音频 I2S | MCLK=42, BCLK=40, WS=39, DOUT=41, DIN=15 | ES8311+ES7210 |
| 音频功放使能 | GPIO4 | PA_EN |
| 音频 I2C | 同触摸总线 | DAC=0x18, ADC=0x40 |
| LED | GPIO43 | 指示灯 |
| SDMMC | CLK=16, CMD=38, D0=17 | SDMMC 1-bit |

---

### 第3层：drv（驱动层）

每个驱动提供 drv_xxx_init()、drv_xxx_read/write/set() 等底层操作。

| 文件 | 目标芯片 | 总线 |
|------|---------|------|
| drv_lcd.c/h | ST77916 | QSPI (40MHz) |
| drv_touch.c/h | CST816S | I2C (100KHz) |
| drv_audio.c/h | ES8311 + ES7210 | I2C + I2S |
| drv_led.c/h | GPIO 控制 | GPIO |
| drv_sdcard.c/h | SD 卡 | SDMMC 1-bit |

---

## 三、依赖的 ESP Registry 组件

- espressif/esp_lcd_touch_cst816s：CST816S 触摸驱动
- espressif/esp_lcd_touch：触摸框架
- espressif/esp_lvgl_port：LVGL 移植层
- lvgl/lvgl v9.3：GUI 引擎
- espressif/esp_lcd_st77916：ST77916 LCD 驱动

---

## 四、中断和关键任务

| 名称 | 优先级 | 用途 |
|------|--------|------|
| pm | 2 | 电源管理任务，500ms 轮询 |
| svc_evt | 3 | 事件总线异步任务 |
| ind | 1 | LED 指示灯控制 |
| Tmr Svc | 1 | FreeRTOS 定时器服务 |
| LVGL GUI | — | LVGL 内部任务，周期性刷新显示 |

