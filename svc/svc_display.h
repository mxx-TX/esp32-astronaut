#ifndef SVC_DISPLAY_H
#define SVC_DISPLAY_H
#include "framework/svc_manager.h"
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch.h"
#include "bsp_touch.h"
#include "lvgl.h"
#include <stdbool.h>

extern svc_base_t g_svc_display;

esp_err_t svc_display_init(esp_lcd_panel_io_handle_t io, esp_lcd_panel_handle_t panel, esp_lcd_touch_handle_t touch_dev);
esp_err_t svc_display_set_backlight(uint8_t pct);
#endif