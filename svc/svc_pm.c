#include "svc_pm.h"
#include "bsp_board.h"
#include "bsp_lcd.h"
#include "bsp_touch.h"
#include "esp_log.h"
#include "esp_sleep.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "bsp_board_cfg.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "svc_pm";

typedef enum { PM_ACTIVE, PM_IDLE, PM_LIGHT_SLEEP } pm_state_t;

static pm_state_t s_state = PM_ACTIVE;
static int64_t s_last_activity = 0;
static bsp_handles_t *s_h = NULL;

#define MS_ACTIVE  15000
#define MS_IDLE    60000
#define MS_LIGHT   180000

void svc_pm_activity(void)
{
    s_last_activity = esp_timer_get_time() / 1000;
    if (s_state != PM_ACTIVE) {
        s_state = PM_ACTIVE;
        bsp_lcd_set_backlight(100);
        ESP_LOGI(TAG, "wake -> ACTIVE");
    }
}

static void pm_task(void *arg)
{
    (void)arg;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        int64_t now = esp_timer_get_time() / 1000;
        int64_t idle = now - s_last_activity;

        /* 检查触摸 */
        if (s_h && s_h->i2c_touch) {
            bsp_touch_data_t t;
            if (bsp_touch_read(s_h->i2c_touch, &t) == ESP_OK && t.pressed) {
                svc_pm_activity();
            }
        }

        switch (s_state) {
        case PM_ACTIVE:
            if (idle >= MS_ACTIVE) {
                s_state = PM_IDLE;
                bsp_lcd_set_backlight(5);
                ESP_LOGI(TAG, "IDLE");
            }
            break;

        case PM_IDLE:
            if (idle >= MS_ACTIVE + MS_IDLE) {
                s_state = PM_LIGHT_SLEEP;
                bsp_lcd_set_backlight(0);
                ESP_LOGI(TAG, "LIGHT_SLEEP");
            }
            break;

        case PM_LIGHT_SLEEP:
            esp_sleep_enable_gpio_wakeup();
            gpio_wakeup_enable(GPIO_NUM_10, GPIO_INTR_LOW_LEVEL);
            esp_light_sleep_start();
            gpio_wakeup_disable(GPIO_NUM_10);
            s_state = PM_ACTIVE;
            s_last_activity = esp_timer_get_time() / 1000;
            bsp_lcd_set_backlight(100);
            ESP_LOGI(TAG, "touch wake -> ACTIVE");
            break;
        }
    }
}

static esp_err_t svc_pm_on_init(svc_base_t *svc, void *ctx)
{
    (void)svc;
    s_h = (bsp_handles_t *)ctx;
    s_last_activity = esp_timer_get_time() / 1000;
    return ESP_OK;
}

static esp_err_t svc_pm_on_start(svc_base_t *svc)
{
    (void)svc;
    xTaskCreate(pm_task, "pm", 3072, NULL, 2, NULL);
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