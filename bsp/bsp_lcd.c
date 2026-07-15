#include "bsp_lcd.h"
#include "bsp_board_cfg.h"
#include "drv_lcd.h"

esp_err_t bsp_lcd_init(esp_lcd_panel_io_handle_t *io, esp_lcd_panel_handle_t *panel)
{
    drv_lcd_cfg_t cfg = {
        .spi_host  = BSP_LCD_SPI_HOST,
        .mosi      = BSP_LCD_MOSI,
        .miso      = BSP_LCD_MISO,
        .sclk      = BSP_LCD_SCLK,
        .quadwp    = BSP_LCD_QWP,
        .quadhd    = BSP_LCD_QHD,
        .cs        = BSP_LCD_CS,
        .dc        = BSP_LCD_DC,
        .reset     = BSP_LCD_RST,
        .bl        = BSP_LCD_BL,
        .bl_timer  = LEDC_TIMER_0,
        .bl_channel = LEDC_CHANNEL_0,
        .hres      = BSP_LCD_WIDTH,
        .vres      = BSP_LCD_HEIGHT,
    };
    return drv_lcd_init(&cfg, io, panel);
}

esp_err_t bsp_lcd_set_backlight(uint8_t pct)
{
    drv_lcd_cfg_t cfg = { .bl = BSP_LCD_BL, .bl_timer = LEDC_TIMER_0, .bl_channel = LEDC_CHANNEL_0 };
    return drv_lcd_set_backlight(&cfg, pct);
}

esp_err_t bsp_lcd_get_resolution(uint16_t *w, uint16_t *h)
{
    drv_lcd_cfg_t cfg = { .hres = BSP_LCD_WIDTH, .vres = BSP_LCD_HEIGHT };
    return drv_lcd_get_resolution(&cfg, w, h);
}
