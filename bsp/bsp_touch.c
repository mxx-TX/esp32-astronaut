#include "bsp_touch.h"
#include "bsp_board_cfg.h"
#include "drv_touch.h"

esp_err_t bsp_touch_init(i2c_master_bus_handle_t i2c_bus, i2c_master_dev_handle_t *out_dev)
{
    drv_touch_cfg_t cfg = { .i2c_bus = i2c_bus, .i2c_addr = BSP_TOUCH_I2C_ADDR, .i2c_speed = BSP_TOUCH_I2C_SPEED };
    return drv_touch_init(&cfg, out_dev);
}

esp_err_t bsp_touch_read(i2c_master_dev_handle_t dev, bsp_touch_data_t *data)
{
    return drv_touch_read(dev, (drv_touch_data_t *)data);
}
