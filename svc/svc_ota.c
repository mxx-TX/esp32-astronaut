#include "svc_ota.h"
#include "svc_http.h"
#include "svc_storage.h"
#include "framework/svc_event.h"
#include "esp_log.h"
#include "esp_https_ota.h"
#include "esp_ota_ops.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "svc_ota";
static const char *VER_URL = "http://bin.bemfa.com/b/374400/version.json";
static const char *NVS_KEY_LVER = "update_version";

static int parse_version(const char *v)
{
    int a = 0, b = 0, c = 0;
    sscanf(v, "%d.%d.%d", &a, &b, &c);
    return a * 10000 + b * 100 + c;
}

esp_err_t svc_ota_start(const char *url)
{
    esp_http_client_config_t http_cfg = {
        .url = url,
        .timeout_ms = 10000,
        .keep_alive_enable = true,
    };
    esp_https_ota_config_t ota_cfg = { .http_config = &http_cfg };

    ESP_LOGI(TAG, "starting OTA from: %s", url);
    esp_err_t ret = esp_https_ota(&ota_cfg);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "OTA failed: 0x%x", ret);
        return ret;
    }
    ESP_LOGI(TAG, "OTA success, rebooting...");
    vTaskDelay(pdMS_TO_TICKS(1000));
    esp_restart();
    return ESP_OK;
}

esp_err_t svc_ota_rollback(void)
{
    ESP_LOGW(TAG, "rollback to previous version");
    esp_ota_mark_app_invalid_rollback_and_reboot();
    return ESP_OK;
}

static void on_network_up(void)
{
    char resp[512];
    size_t len = 0;

    esp_err_t ret = svc_http_download(VER_URL, (uint8_t *)resp, &len, sizeof(resp) - 1);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "check version fail: 0x%x", ret);
        return;
    }
    resp[len] = 0;
    ESP_LOGI(TAG, "version response: %s", resp);

    char *p = strstr(resp, "\"version\":\"");
    char *q = strstr(resp, "\"url\":\"");
    if (!p || !q) {
        ESP_LOGW(TAG, "bad version response");
        return;
    }

    p += 10;
    char *end = strchr(p, '"');
    if (!end) return;
    *end = 0;

    int new_ver = parse_version(p);
    if (new_ver == 0) { ESP_LOGW(TAG, "bad version: %s", p); return; }

    const esp_app_desc_t *app = esp_app_get_description();
    int cur_ver = parse_version(app->version);
    ESP_LOGI(TAG, "cur=%s(%d) new=%s(%d)", app->version, cur_ver, p, new_ver);

    char saved[16] = {0};
    size_t slen = sizeof(saved);
    svc_storage_nvs_get_str(NVS_KEY_LVER, saved, &slen);
    if (new_ver <= parse_version(saved)) { ESP_LOGI(TAG, "already updated"); return; }
    if (new_ver <= cur_ver) { ESP_LOGI(TAG, "no newer version"); return; }

    q += 6;
    end = strchr(q, '"');
    if (!end) return;
    *end = 0;

    ESP_LOGI(TAG, "new version %s found, start OTA...", p);
    ret = svc_ota_start(q);
    if (ret == ESP_OK) {
        svc_storage_nvs_set_str(NVS_KEY_LVER, p);
    } else {
        ESP_LOGE(TAG, "OTA failed: 0x%x", ret);
    }
}

static void net_event_handler(svc_event_id_t id, void *data, void *ctx)
{
    (void)data; (void)ctx;
    if (id == EVT_NETWORK_UP) {
        ESP_LOGI(TAG, "network up, check update...");
        on_network_up();
    }
}

static esp_err_t svc_ota_on_init(svc_base_t *svc, void *ctx)
{
    (void)svc; (void)ctx;
    esp_ota_mark_app_valid_cancel_rollback();
    return ESP_OK;
}

static esp_err_t svc_ota_on_start(svc_base_t *svc)
{
    (void)svc;
    svc_event_subscribe(EVT_NETWORK_UP, net_event_handler, NULL);
    return ESP_OK;
}

static const char *s_ota_deps[] = {"wifi", "http"};

svc_base_t g_svc_ota = {
    .name = "ota",
    .deps = s_ota_deps,
    .dep_count = 2,
    .on_init = svc_ota_on_init,
    .on_start = svc_ota_on_start,
    .on_stop = NULL,
    .on_deinit = NULL,
};
