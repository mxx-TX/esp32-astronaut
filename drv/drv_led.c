#include "drv_led.h"
#include "driver/gpio.h"

static int s_gpio = -1;

esp_err_t drv_led_init(const drv_led_cfg_t *cfg)
{
    s_gpio = cfg->gpio;
    gpio_config_t c = {
        .pin_bit_mask = (1ULL << s_gpio),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
    };
    esp_err_t ret = gpio_config(&c);
    if (ret == ESP_OK) {
        gpio_set_level(s_gpio, 1);
    }
    return ret;
}

esp_err_t drv_led_on(void)
{
    return gpio_set_level(s_gpio, 0);
}

esp_err_t drv_led_off(void)
{
    return gpio_set_level(s_gpio, 1);
}

esp_err_t drv_led_toggle(void)
{
    int level = gpio_get_level(s_gpio);
    return gpio_set_level(s_gpio, level ? 0 : 1);
}
