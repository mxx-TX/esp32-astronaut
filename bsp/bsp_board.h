#ifndef BSP_BOARD_H
#define BSP_BOARD_H
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch.h"
#include "driver/i2s_std.h"
#include "driver/i2c_master.h"

typedef struct {
    esp_lcd_panel_io_handle_t lcd_io;
    esp_lcd_panel_handle_t lcd_panel;
    i2c_master_bus_handle_t i2c_bus;
    esp_lcd_touch_handle_t i2c_touch;
    i2c_master_dev_handle_t i2c_dac;
    i2c_master_dev_handle_t i2c_adc;
    i2s_chan_handle_t i2s_tx;
    i2s_chan_handle_t i2s_rx;
} bsp_handles_t;

esp_err_t bsp_board_init(bsp_handles_t *handles);
esp_err_t bsp_board_lcd_power_on(void);
esp_err_t bsp_board_audio_power_on(void);

#endif