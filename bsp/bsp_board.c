#include "bsp_board.h"
#include "bsp_board_cfg.h"
#include "bsp_lcd.h"
#include "bsp_touch.h"
#include "bsp_audio.h"
#include "bsp_sdcard.h"
#include "bsp_led.h"
#include "os_task.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "bsp_board";

static void power_gpio_init(void)
{
    gpio_config_t cfg_lcd = {
        .pin_bit_mask = (1ULL << BSP_GPIO_LCD_EN),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&cfg_lcd); gpio_set_level(BSP_GPIO_LCD_EN, 0);
    gpio_config_t cfg_pa  = { .pin_bit_mask = (1ULL << BSP_GPIO_PA_EN), .mode = GPIO_MODE_OUTPUT };
    gpio_config(&cfg_pa);  gpio_set_level(BSP_GPIO_PA_EN, 0);
}

esp_err_t bsp_board_audio_power_on(void)
{
    gpio_set_level(BSP_GPIO_PA_EN, 1);
    return ESP_OK;
}

esp_err_t bsp_board_init(bsp_handles_t *h)
{
    memset(h, 0, sizeof(*h));
    power_gpio_init();
    vTaskDelay(pdMS_TO_TICKS(100));

    // I2C master bus
    i2c_master_bus_config_t i2c_cfg = {
        .i2c_port = BSP_I2C_PORT,
        .sda_io_num = BSP_I2C_SDA,
        .scl_io_num = BSP_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = false,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_cfg, &h->i2c_bus));
    ESP_LOGI(TAG, "I2C init ok");

    // LCD
    ESP_ERROR_CHECK(bsp_lcd_init(&h->lcd_io, &h->lcd_panel));
    bsp_lcd_set_backlight(100);
    ESP_LOGI(TAG, "LCD init ok");

    // Touch - optional
    esp_err_t ret = bsp_touch_init(h->i2c_bus, &h->i2c_touch);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Touch init skip: 0x%x", ret);
        h->i2c_touch = NULL;
    }

    // Audio - optional, ES8311/ES7210 可能不在板上
    ret = bsp_audio_init(h->i2c_bus, &h->i2c_dac, &h->i2c_adc, &h->i2s_tx, &h->i2s_rx);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Audio init skip: 0x%x", ret);
        h->i2c_dac = NULL;
        h->i2c_adc = NULL;
        h->i2s_tx = NULL;
        h->i2s_rx = NULL;
    }

    // SDCard - optional
    ret = bsp_sdcard_init();
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "SDCard init skip: 0x%x", ret);
    }

    // LED
    ESP_ERROR_CHECK(bsp_led_init());
    ESP_LOGI(TAG, "LED init ok");

    return ESP_OK;
}