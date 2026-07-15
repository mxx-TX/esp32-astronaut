#include "svc_display.h"
#include "bsp_board.h"
#include "bsp_lcd.h"
#include "bsp_touch.h"
#include "esp_log.h"
#include "esp_heap_caps.h"
static const char *TAG = "svc_display";
static esp_lcd_panel_handle_t s_panel = NULL;
static i2c_master_dev_handle_t s_touch = NULL;
static lv_display_t *s_disp = NULL;

esp_err_t svc_display_init(esp_lcd_panel_handle_t panel, i2c_master_dev_handle_t touch_dev)
{
    s_panel = panel;
    s_touch = touch_dev;
    lv_init();
    uint16_t w, h;
    bsp_lcd_get_resolution(&w, &h);
    size_t buf_sz = w * 40 * sizeof(lv_color16_t);
    lv_color16_t *buf1 = heap_caps_malloc(buf_sz, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    s_disp = lv_display_create(w, h);
    lv_display_set_flush_cb(s_disp, svc_display_flush);
    lv_display_set_buffers(s_disp, buf1, NULL, buf_sz, LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_color_format(s_disp, LV_COLOR_FORMAT_RGB565);
    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, svc_display_touch_read);
    ESP_LOGI(TAG, "LVGL init ok, buf=%u bytes each", buf_sz);
    return ESP_OK;
}
void svc_display_flush(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    (void)disp;
    esp_lcd_panel_draw_bitmap(s_panel, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map);
    lv_display_flush_ready(disp);
}
void svc_display_touch_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    bsp_touch_data_t t;
    if (s_touch && bsp_touch_read(s_touch, &t) == ESP_OK && t.pressed) {
        data->point.x = t.x;
        data->point.y = t.y;
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}
esp_err_t svc_display_set_backlight(uint8_t pct) { return bsp_lcd_set_backlight(pct); }

static esp_err_t svc_display_on_init(svc_base_t *svc, void *ctx)
{
    bsp_handles_t *h = (bsp_handles_t *)ctx;
    return svc_display_init(h->lcd_panel, h->i2c_touch);
}

svc_base_t g_svc_display = {
    .name = "display",
    .deps = NULL,
    .dep_count = 0,
    .on_init = svc_display_on_init,
    .on_start = NULL,
    .on_stop = NULL,
    .on_deinit = NULL,
};
