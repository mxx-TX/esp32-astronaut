#ifndef BSP_LED_H
#define BSP_LED_H
#include "esp_err.h"

esp_err_t bsp_led_init(void);
esp_err_t bsp_led_on(void);
esp_err_t bsp_led_off(void);
esp_err_t bsp_led_toggle(void);

#endif
