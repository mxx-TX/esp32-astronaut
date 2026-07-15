#ifndef DRV_LED_H
#define DRV_LED_H
#include "esp_err.h"
#include <stdint.h>

typedef struct {
    int gpio;
} drv_led_cfg_t;

esp_err_t drv_led_init(const drv_led_cfg_t *cfg);
esp_err_t drv_led_on(void);
esp_err_t drv_led_off(void);
esp_err_t drv_led_toggle(void);

#endif
