#ifndef BSP_TOUCH_H
#define BSP_TOUCH_H
#include "esp_err.h"
#include "driver/i2c_master.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    uint16_t x;
    uint16_t y;
    bool pressed;
} bsp_touch_data_t;

esp_err_t bsp_touch_init(i2c_master_bus_handle_t i2c_bus, i2c_master_dev_handle_t *out_dev);
esp_err_t bsp_touch_read(i2c_master_dev_handle_t dev, bsp_touch_data_t *data);

#endif
