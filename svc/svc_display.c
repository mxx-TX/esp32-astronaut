#include "svc_display.h"
#include "bsp_board.h"
#include "bsp_lcd.h"
#include "bsp_touch.h"
#include "esp_lvgl_port.h"
#include "esp_log.h"

static const char *TAG = "svc_display";
static lv_display_t *s_disp = NULL;

esp_err_t svc_display_init(esp_lcd_panel_io_handle_t io, esp_lcd_panel_handle_t panel, esp_lcd_touch_handle_t touch_dev)
{
    const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
    esp_err_t ret = lvgl_port_init(&lvgl_cfg);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "lvgl port init fail: 0x%x", ret); return ret; }

    uint16_t w, h;
    bsp_lcd_get_resolution(&w, &h);

    const lvgl_port_display_cfg_t disp_cfg = {
        .io_handle = io,
       .panel_handle = panel,
        .buffer_size = w * 80,
       .double_buffer = false,
        .hres = w,
        .vres = h,
        .monochrome = false,
        .rotation = { .swap_xy = false, .mirror_x = false, .mirror_y = false },
#if LVGL_VERSION_MAJOR >= 9
        .color_format = LV_COLOR_FORMAT_RGB565,
#endif
        .flags = { .swap_bytes = true, .buff_dma = true },
    };
    s_disp = lvgl_port_add_disp(&disp_cfg);
    if (s_disp == NULL) { ESP_LOGE(TAG, "lvgl add disp fail"); return ESP_FAIL; }

    if (touch_dev) {
        const lvgl_port_touch_cfg_t touch_cfg = {
            .disp = s_disp,
            .handle = touch_dev,
        };
        lv_indev_t *indev = lvgl_port_add_touch(&touch_cfg);
        if (indev == NULL) { ESP_LOGW(TAG, "lvgl add touch fail (continue)"); }
    }

    ESP_LOGI(TAG, "LVGL port init ok, %ux%u", w, h);
    return ESP_OK;
}

esp_err_t svc_display_set_backlight(uint8_t pct) 
{ 
    return bsp_lcd_set_backlight(pct); 

}

static esp_err_t svc_display_on_init(svc_base_t *svc, void *ctx)
{
    bsp_handles_t *h = (bsp_handles_t *)ctx;
    return svc_display_init(h->lcd_io, h->lcd_panel, h->i2c_touch);
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
