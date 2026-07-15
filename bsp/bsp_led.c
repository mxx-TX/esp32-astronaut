#include "bsp_led.h"
#include "bsp_board_cfg.h"
#include "drv_led.h"

esp_err_t bsp_led_init(void)
{
    drv_led_cfg_t cfg = { .gpio = BSP_LED_GPIO };
    return drv_led_init(&cfg);
}

esp_err_t bsp_led_on(void)   { return drv_led_on(); }
esp_err_t bsp_led_off(void)  { return drv_led_off(); }
esp_err_t bsp_led_toggle(void) { return drv_led_toggle(); }
