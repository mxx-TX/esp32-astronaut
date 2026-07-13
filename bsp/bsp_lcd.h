#ifndef BSP_LCD_H
#define BSP_LCD_H
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"

esp_err_t bsp_lcd_init(esp_lcd_panel_io_handle_t *out_io, esp_lcd_panel_handle_t *out_panel);
esp_err_t bsp_lcd_set_backlight(uint8_t percent);
esp_err_t bsp_lcd_get_resolution(uint16_t *width, uint16_t *height);

#endif
