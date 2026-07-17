#ifndef DRV_TOUCH_H
#define DRV_TOUCH_H
#include "esp_err.h"
#include "driver/i2c_master.h"
#include "esp_lcd_touch.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    i2c_master_bus_handle_t i2c_bus;
    uint8_t i2c_addr;
    uint32_t i2c_speed;
    uint16_t x_max;
    uint16_t y_max;
    gpio_num_t int_gpio;
    gpio_num_t rst_gpio;
} drv_touch_cfg_t;

typedef struct {
    uint16_t x;
    uint16_t y;
    bool pressed;
} drv_touch_data_t;

esp_err_t drv_touch_init(const drv_touch_cfg_t *cfg, esp_lcd_touch_handle_t *out_tp);
esp_err_t drv_touch_read(esp_lcd_touch_handle_t tp, drv_touch_data_t *data);
esp_err_t drv_touch_deinit(esp_lcd_touch_handle_t tp);

#endif
