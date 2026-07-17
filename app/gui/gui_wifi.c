#include "gui_wifi.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "esp_log.h"
#include "framework/svc_event.h"
#include "svc_wifi.h"
#include "svc_storage.h"
#include <string.h>

static const char *TAG = "gui_wifi";
static lv_obj_t *s_popup = NULL;

static void show_popup(const char *msg)
{
    if (s_popup) {
        lv_obj_del(s_popup);
    }
    s_popup = lv_label_create(lv_screen_active());
    lv_label_set_text(s_popup, msg);
    lv_obj_set_style_bg_color(s_popup, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(s_popup, LV_OPA_COVER, 0);
    lv_obj_set_style_text_color(s_popup, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_pad_all(s_popup, 10, 0);
    lv_obj_align(s_popup, LV_ALIGN_TOP_MID, 0, 50);
}

static void hide_popup(void)
{
    if (!s_popup) return;
    lv_obj_del(s_popup);
    s_popup = NULL;
}

static void update_ssid_label(void)
{
    char ssid[33] = {0};
    size_t len = sizeof(ssid);
    if (svc_storage_nvs_get_str("wifi_ssid", ssid, &len) != ESP_OK) return;
    if (ssid[0] == 0) return;

    extern lv_obj_t * uic_WifiSsidText;
    if (uic_WifiSsidText) {
        lv_label_set_text(uic_WifiSsidText, ssid);
    }
}

static void wifi_event_handler(svc_event_id_t id, void *data, void *ctx)
{
    (void)data; (void)ctx;

    lvgl_port_lock(0);
    switch (id) {
    case EVT_WIFI_PROV_STARTED:
        break;
    case EVT_WIFI_CONNECTING:
        show_popup("Connecting to WiFi...");
        break;
    case EVT_NETWORK_UP:
        show_popup("WiFi connected");
        update_ssid_label();
        break;
    case EVT_NETWORK_DOWN:
        show_popup("WiFi disconnected");
        break;
    default:
        break;
    }
    lvgl_port_unlock();

    if (id == EVT_NETWORK_UP) {
        vTaskDelay(pdMS_TO_TICKS(2000));
        lvgl_port_lock(0);
        hide_popup();
        lvgl_port_unlock();
    }
}

void gui_wifi_show_message(const char *msg)
{
    lvgl_port_lock(0);
    show_popup(msg);
    lvgl_port_unlock();
}

void gui_wifi_hide_message(void)
{
    lvgl_port_lock(0);
    hide_popup();
    lvgl_port_unlock();
}

void gui_wifi_init(void)
{
    svc_event_subscribe(EVT_WIFI_PROV_STARTED, wifi_event_handler, NULL);
    svc_event_subscribe(EVT_WIFI_CONNECTING, wifi_event_handler, NULL);
    svc_event_subscribe(EVT_NETWORK_UP, wifi_event_handler, NULL);
    svc_event_subscribe(EVT_NETWORK_DOWN, wifi_event_handler, NULL);
    ESP_LOGI(TAG, "gui_wifi init ok");
}
