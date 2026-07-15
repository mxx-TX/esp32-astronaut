#ifndef DRV_TOUCH_H
#define DRV_TOUCH_H
#include "esp_err.h"
#include "driver/i2c_master.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct {
    i2c_master_bus_handle_t i2c_bus;
    uint8_t i2c_addr;
    uint32_t i2c_speed;
} drv_touch_cfg_t;

typedef struct {
    uint16_t x;
    uint16_t y;
    bool pressed;
} drv_touch_data_t;

esp_err_t drv_touch_init(const drv_touch_cfg_t *cfg, i2c_master_dev_handle_t *out_dev);
esp_err_t drv_touch_read(i2c_master_dev_handle_t dev, drv_touch_data_t *data);

#endif
