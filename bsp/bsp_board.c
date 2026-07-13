#include "bsp_board.h"
#include "bsp_lcd.h"
#include "bsp_touch.h"
#include "bsp_audio.h"
#include "bsp_sdcard.h"
#include "bsp_led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
static const char *TAG = "bsp_board";
static void power_gpio_init(void)
{
    gpio_config_t cfg_lcd = { .pin_bit_mask = (1ULL << 9),  .mode = GPIO_MODE_OUTPUT };
    gpio_config_t cfg_aud = { .pin_bit_mask = (1ULL << 48), .mode = GPIO_MODE_OUTPUT };
    gpio_config_t cfg_pa  = { .pin_bit_mask = (1ULL << 15), .mode = GPIO_MODE_OUTPUT };
    gpio_config(&cfg_lcd); gpio_set_level(9, 1);
    gpio_config(&cfg_aud); gpio_set_level(48, 0);
    gpio_config(&cfg_pa);  gpio_set_level(15, 0);
}
esp_err_t bsp_board_lcd_power_on(void)
{
    gpio_set_level(9, 0);
    return ESP_OK;
}
esp_err_t bsp_board_audio_power_on(void)
{
    gpio_set_level(48, 1);
    return ESP_OK;
}
esp_err_t bsp_board_init(bsp_handles_t *h)
{
    esp_err_t ret;
    memset(h, 0, sizeof(*h));
    power_gpio_init();
    ESP_LOGI(TAG, "LCD power on");
    bsp_board_lcd_power_on();
    vTaskDelay(pdMS_TO_TICKS(50));

    ESP_LOGI(TAG, "Init I2C bus");
    i2c_master_bus_config_t i2c_cfg = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = 2,
        .scl_io_num = 1,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = { .enable_internal_pullup = true },
    };
    ret = i2c_new_master_bus(&i2c_cfg, &h->i2c_bus);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "I2C bus fail"); return ret; }

    ESP_LOGI(TAG, "Init LCD");
    ret = bsp_lcd_init(&h->lcd_io, &h->lcd_panel);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "LCD fail"); return ret; }
    bsp_lcd_set_backlight(80);

    ESP_LOGI(TAG, "Init Touch");
    ret = bsp_touch_init(h->i2c_bus, &h->i2c_touch);
    if (ret != ESP_OK) { ESP_LOGW(TAG, "Touch fail (continue)"); }

    ESP_LOGI(TAG, "Audio power on");
    bsp_board_audio_power_on();
    ret = bsp_audio_init(h->i2c_bus, &h->i2c_dac, &h->i2c_adc, &h->i2s_tx, &h->i2s_rx);
    if (ret != ESP_OK) { ESP_LOGW(TAG, "Audio fail (continue)"); }

    ESP_LOGI(TAG, "Init LED");
    bsp_led_init();

    ESP_LOGI(TAG, "Init SD card");
    ret = bsp_sdcard_init();
    if (ret != ESP_OK) { ESP_LOGW(TAG, "SD card fail (continue)"); }

    ESP_LOGI(TAG, "Board init complete");
    return ESP_OK;
}
