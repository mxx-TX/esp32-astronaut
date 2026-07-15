#include "test_drv.h"
#include "drv_lcd.h"
#include "drv_touch.h"
#include "drv_audio.h"
#include "drv_led.h"

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "test_drv";

#define T_CHECK(name, expr) do { \
    esp_err_t r = (expr); \
    ESP_LOGI(TAG, "[%s] %s", (r == ESP_OK) ? "PASS" : "FAIL", name); \
} while(0)

static void test_drv_led_ops(void)
{
    ESP_LOGI(TAG, "--- LED Ops ---");
    T_CHECK("led on", drv_led_on());
    vTaskDelay(pdMS_TO_TICKS(100));
    T_CHECK("led off", drv_led_off());
    vTaskDelay(pdMS_TO_TICKS(100));
    T_CHECK("led toggle", drv_led_toggle());
    vTaskDelay(pdMS_TO_TICKS(100));
    T_CHECK("led toggle again", drv_led_toggle());
}

static void test_drv_lcd_ops(void)
{
    ESP_LOGI(TAG, "--- LCD Ops ---");
    drv_lcd_cfg_t cfg = { .bl = 44, .bl_timer = LEDC_TIMER_0, .bl_channel = LEDC_CHANNEL_0,
                           .hres = 360, .vres = 360 };
    T_CHECK("backlight 100", drv_lcd_set_backlight(&cfg, 100));
    vTaskDelay(pdMS_TO_TICKS(200));
    T_CHECK("backlight 50", drv_lcd_set_backlight(&cfg, 50));
    vTaskDelay(pdMS_TO_TICKS(200));
    T_CHECK("backlight 10", drv_lcd_set_backlight(&cfg, 10));
    vTaskDelay(pdMS_TO_TICKS(200));
    T_CHECK("backlight 80", drv_lcd_set_backlight(&cfg, 80));
    uint16_t w = 0, h = 0;
    T_CHECK("get resolution", drv_lcd_get_resolution(&cfg, &w, &h));
    ESP_LOGI(TAG, "  resolution: %ux%u", w, h);
}

static void test_drv_touch_read(i2c_master_dev_handle_t dev)
{
    ESP_LOGI(TAG, "--- Touch Ops ---");
    drv_touch_data_t t;
    esp_err_t r = drv_touch_read(dev, &t);
    if (r == ESP_OK) {
        ESP_LOGI(TAG, "[PASS] touch read x=%u y=%u pressed=%d", t.x, t.y, t.pressed);
    } else {
        ESP_LOGW(TAG, "[WARN] touch read fail: 0x%x", r);
    }
}

static void test_drv_audio_ops(i2c_master_dev_handle_t dac, i2c_master_dev_handle_t adc)
{
    ESP_LOGI(TAG, "--- Audio Ops ---");
    T_CHECK("set vol 50", drv_audio_set_volume(dac, 50));
    T_CHECK("set vol 0", drv_audio_set_volume(dac, 0));
    T_CHECK("set vol 80", drv_audio_set_volume(dac, 80));
    T_CHECK("mic gain 0dB", drv_audio_set_mic_gain(adc, 0));
    T_CHECK("mic gain 10dB", drv_audio_set_mic_gain(adc, 10));
    T_CHECK("mic gain 30dB", drv_audio_set_mic_gain(adc, 30));
}

esp_err_t test_drv_run_all(esp_lcd_panel_handle_t lcd_panel,
                            i2c_master_dev_handle_t touch_dev,
                            i2c_master_dev_handle_t dac_dev,
                            i2c_master_dev_handle_t adc_dev)
{
    ESP_LOGI(TAG, "========== DRV Test Suite ==========");
    test_drv_led_ops();
    test_drv_lcd_ops();
    if (touch_dev) { test_drv_touch_read(touch_dev); }
    if (dac_dev && adc_dev) { test_drv_audio_ops(dac_dev, adc_dev); }
    ESP_LOGI(TAG, "========== DRV Test Done ==========");
    return ESP_OK;
}
