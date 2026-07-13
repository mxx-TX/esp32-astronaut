#include "bsp_led.h"
#include "driver/gpio.h"

#define BSP_LED_GPIO 43

esp_err_t bsp_led_init(void)
{
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << BSP_LED_GPIO),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    esp_err_t ret = gpio_config(&cfg);
    if (ret == ESP_OK) {
        gpio_set_level(BSP_LED_GPIO, 1);
    }
    return ret;
}

esp_err_t bsp_led_on(void)
{
    return gpio_set_level(BSP_LED_GPIO, 0);
}

esp_err_t bsp_led_off(void)
{
    return gpio_set_level(BSP_LED_GPIO, 1);
}

esp_err_t bsp_led_toggle(void)
{
    int level = gpio_get_level(BSP_LED_GPIO);
    return gpio_set_level(BSP_LED_GPIO, level ? 0 : 1);
}
