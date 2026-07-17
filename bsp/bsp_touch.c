#include "bsp_touch.h"
#include "bsp_board_cfg.h"
#include "drv_touch.h"

esp_err_t bsp_touch_init(i2c_master_bus_handle_t i2c_bus, esp_lcd_touch_handle_t *out_tp)
{
    drv_touch_cfg_t cfg = {
        .i2c_bus = i2c_bus,
        .i2c_addr = BSP_TOUCH_I2C_ADDR,
        .i2c_speed = BSP_TOUCH_I2C_SPEED,
        .x_max = BSP_LCD_WIDTH,
        .y_max = BSP_LCD_HEIGHT,
        .int_gpio = BSP_TOUCH_INT_GPIO,
        .rst_gpio = BSP_TOUCH_RST_GPIO,
    };
    return drv_touch_init(&cfg, out_tp);
}

esp_err_t bsp_touch_read(esp_lcd_touch_handle_t tp, bsp_touch_data_t *data)
{
    return drv_touch_read(tp, (drv_touch_data_t *)data);
}
