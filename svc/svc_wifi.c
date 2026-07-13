#include "svc_wifi.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <string.h>
static const char *TAG = "svc_wifi";
static bool s_connected = false;
static svc_wifi_cb_t s_cb = NULL;
static EventGroupHandle_t s_evt;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1
static void event_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    if (base == WIFI_EVENT) {
        if (id == WIFI_EVENT_STA_START) {
            esp_wifi_connect();
        } else if (id == WIFI_EVENT_STA_DISCONNECTED) {
            s_connected = false;
            if (s_cb) s_cb(false);
            ESP_LOGW(TAG, "WiFi disconnected, retry...");
            esp_wifi_connect();
        }
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        s_connected = true;
        if (s_cb) s_cb(true);
        xEventGroupSetBits(s_evt, WIFI_CONNECTED_BIT);
        ESP_LOGI(TAG, "Got IP");
    }
}
esp_err_t svc_wifi_init(void)
{
    s_evt = xEventGroupCreate();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, event_handler, NULL, NULL);
    esp_wifi_set_mode(WIFI_MODE_STA);
    return ESP_OK;
}
esp_err_t svc_wifi_connect(const char *ssid, const char *pass, svc_wifi_cb_t cb)
{
    s_cb = cb;
    wifi_config_t cfg = {0};
    strncpy((char *)cfg.sta.ssid, ssid, 32);
    strncpy((char *)cfg.sta.password, pass, 64);
    esp_wifi_set_config(WIFI_IF_STA, &cfg);
    esp_wifi_start();
    EventBits_t bits = xEventGroupWaitBits(s_evt, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdFALSE, pdFALSE, pdMS_TO_TICKS(15000));
    if (bits & WIFI_CONNECTED_BIT) return ESP_OK;
    return ESP_FAIL;
}
esp_err_t svc_wifi_disconnect(void) { esp_wifi_stop(); s_connected = false; return ESP_OK; }
bool svc_wifi_is_connected(void) { return s_connected; }
