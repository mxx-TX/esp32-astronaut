#ifndef SVC_DISPLAY_H
#define SVC_DISPLAY_H
#include "esp_err.h"
#include "esp_lcd_panel_ops.h"
#include "bsp_touch.h"
#include "lvgl.h"
#include <stdbool.h>
esp_err_t svc_display_init(esp_lcd_panel_handle_t panel, i2c_master_dev_handle_t touch_dev);
void svc_display_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map);
void svc_display_touch_read(lv_indev_t *indev, lv_indev_data_t *data);
esp_err_t svc_display_set_backlight(uint8_t pct);
#endif
