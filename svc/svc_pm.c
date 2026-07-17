#include "svc_pm.h"
#include "svc_pm_rtc.h"
#include "bsp_board_cfg.h"
#include "bsp_lcd.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "framework/svc_event.h"
#include "esp_rom_crc.h"
#include "esp_pm.h"
#include <stddef.h>

static const char *TAG = "svc_pm";

typedef enum { PM_ACTIVE, PM_IDLE, PM_LIGHT_SLEEP } pm_state_t;

static pm_state_t s_state = PM_ACTIVE;
static int64_t s_last_activity = 0;

#define MS_ACTIVE  15000
#define MS_IDLE    60000
#define MS_DEEP    (30 * 60 * 1000)

RTC_DATA_ATTR pm_rtc_data_t g_pm_rtc = {0};

void svc_pm_activity(void)
{
    s_last_activity = esp_timer_get_time() / 1000;
    if (s_state != PM_ACTIVE) {
        s_state = PM_ACTIVE;
        bsp_lcd_set_backlight(100);
        svc_event_publish(EVT_PM_WAKE, NULL);
        ESP_LOGI(TAG, "wake -> ACTIVE");
    }
}

void svc_pm_request_deep_sleep(void)
{
    s_last_activity = 0;
}

static void rtc_save(void)
{
    g_pm_rtc.magic = PM_RTC_MAGIC;
    g_pm_rtc.boot_mode = 1;
    g_pm_rtc.wakeup_count++;
    g_pm_rtc.crc32 = esp_rom_crc32_le(UINT32_MAX,
        (const uint8_t *)&g_pm_rtc,
        offsetof(pm_rtc_data_t, crc32));
}

static void enter_deep_sleep(void)
{
    ESP_LOGI(TAG, "DEEP_SLEEP");
    svc_event_publish(EVT_PM_DEEP_SLEEP, NULL);
    rtc_save();
    esp_sleep_enable_ext0_wakeup(BSP_TOUCH_INT_GPIO, 0);
    esp_deep_sleep_start();
}

static void set_pm_mode(bool high_perf)
{
#if CONFIG_PM_ENABLE
    esp_pm_config_t pm_cfg = {
        .max_freq_mhz = 240,
        .min_freq_mhz = high_perf ? 240 : 40,
        .light_sleep_enable = !high_perf,
    };
    esp_pm_configure(&pm_cfg);
#else
    (void)high_perf;
#endif
}

static void pm_task(void *arg)
{
    (void)arg;

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10));

        int64_t now = esp_timer_get_time() / 1000;
        int64_t idle = now - s_last_activity;

        if (gpio_get_level(BSP_TOUCH_INT_GPIO) == 0) {
            svc_pm_activity();
        }

        switch (s_state) {
        case PM_ACTIVE:
            if (idle >= MS_ACTIVE) {
                s_state = PM_IDLE;
                bsp_lcd_set_backlight(5);
                set_pm_mode(false);
                svc_event_publish(EVT_PM_IDLE, NULL);
                ESP_LOGI(TAG, "IDLE");
            }
            break;

        case PM_IDLE:
            if (idle >= MS_ACTIVE + MS_IDLE) {
                s_state = PM_LIGHT_SLEEP;
                bsp_lcd_set_backlight(0);
                svc_event_publish(EVT_PM_SLEEP, NULL);
                ESP_LOGI(TAG, "LIGHT_SLEEP");

                esp_err_t ext0_ret = esp_sleep_enable_ext0_wakeup(BSP_TOUCH_INT_GPIO, 0);
                if (ext0_ret != ESP_OK) {
                    ESP_LOGE(TAG, "ext0 wakeup fail: 0x%x", ext0_ret);
                }
                esp_sleep_enable_timer_wakeup((uint64_t)MS_DEEP * 1000);
                esp_err_t sleep_ret = esp_light_sleep_start();
                ESP_LOGI(TAG, "light sleep return: 0x%x", sleep_ret);

                if (esp_sleep_get_wakeup_causes() & BIT(ESP_SLEEP_WAKEUP_TIMER)) {
                    ESP_LOGI(TAG, "timer wake -> deep sleep");
                    enter_deep_sleep();
                }

                s_last_activity = esp_timer_get_time() / 1000;
                s_state = PM_ACTIVE;
                bsp_lcd_set_backlight(100);
                set_pm_mode(true);
                svc_event_publish(EVT_PM_WAKE, NULL);
                ESP_LOGI(TAG, "touch wake -> ACTIVE");
            }
            break;

        default:
            break;
        }
    }
}

static esp_err_t svc_pm_on_init(svc_base_t *svc, void *ctx)
{
    (void)svc;
    (void)ctx;
    s_last_activity = esp_timer_get_time() / 1000;
    gpio_pullup_en(BSP_TOUCH_INT_GPIO);
    return ESP_OK;
}

static esp_err_t svc_pm_on_start(svc_base_t *svc)
{
    (void)svc;
    xTaskCreate(pm_task, "pm", 3072, NULL, 2, NULL);
    set_pm_mode(true);
    return ESP_OK;
}

svc_base_t g_svc_pm = {
    .name = "pm",
    .deps = NULL,
    .dep_count = 0,
    .on_init = svc_pm_on_init,
    .on_start = svc_pm_on_start,
    .on_stop = NULL,
    .on_deinit = NULL,
};