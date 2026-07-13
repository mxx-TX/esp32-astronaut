#include "test_bsp.h"
#include "bsp_lcd.h"
#include "bsp_touch.h"
#include "bsp_led.h"
#include "bsp_sdcard.h"
#include "bsp_audio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static const char *TAG = "test_bsp";
#define T_CHECK(name, expr) do { esp_err_t r = (expr); ESP_LOGI(TAG, "[%s] %s", (r == ESP_OK) ? "PASS" : "FAIL", name); } while(0)
esp_err_t test_bsp_run_all(bsp_handles_t *h)
{
    ESP_LOGI(TAG, "========== BSP Test Suite ==========");

    T_CHECK("LED on", bsp_led_on());
    vTaskDelay(pdMS_TO_TICKS(200));
    T_CHECK("LED off", bsp_led_off());
    vTaskDelay(pdMS_TO_TICKS(200));
    T_CHECK("LED toggle", bsp_led_toggle());

    T_CHECK("LCD backlight 100", bsp_lcd_set_backlight(100));
    vTaskDelay(pdMS_TO_TICKS(300));
    T_CHECK("LCD backlight 30", bsp_lcd_set_backlight(30));

    bsp_touch_data_t t;
    esp_err_t tr = bsp_touch_read(h->i2c_touch, &t);
    if (tr == ESP_OK) {
        ESP_LOGI(TAG, "[PASS] Touch read x=%u y=%u pressed=%d", t.x, t.y, t.pressed);
    } else {
        ESP_LOGW(TAG, "[WARN] Touch read fail (no HW?): 0x%x", tr);
    }

    esp_err_t sr = bsp_sdcard_init();
    if (sr == ESP_OK) {
        ESP_LOGI(TAG, "[PASS] SD card mounted");
    } else {
        ESP_LOGW(TAG, "[WARN] SD card fail (no card?)");
    }

    T_CHECK("Audio set vol 50", bsp_audio_set_volume(h->i2c_dac, 50));
    T_CHECK("Audio mic gain 10dB", bsp_audio_set_mic_gain(h->i2c_adc, 10));

    ESP_LOGI(TAG, "========== BSP Test Done ==========");
    return ESP_OK;
}
