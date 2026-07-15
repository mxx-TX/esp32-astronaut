#ifndef DRV_LCD_H
#define DRV_LCD_H
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "hal/ledc_types.h"
#include <stdint.h>

typedef struct {
    int spi_host;
    int mosi;
    int miso;
    int sclk;
    int quadwp;
    int quadhd;
    int cs;
    int dc;
    int reset;
    int bl;
    ledc_timer_t bl_timer;
    ledc_channel_t bl_channel;
    uint16_t hres;
    uint16_t vres;
} drv_lcd_cfg_t;

esp_err_t drv_lcd_init(const drv_lcd_cfg_t *cfg, esp_lcd_panel_io_handle_t *out_io, esp_lcd_panel_handle_t *out_panel);
esp_err_t drv_lcd_set_backlight(const drv_lcd_cfg_t *cfg, uint8_t percent);
esp_err_t drv_lcd_get_resolution(const drv_lcd_cfg_t *cfg, uint16_t *width, uint16_t *height);

#endif
