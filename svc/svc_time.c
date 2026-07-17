#include "svc_time.h"
#include "esp_log.h"
#include "esp_netif_sntp.h"
#include <time.h>

static const char *TAG = "svc_time";
static bool s_synced = false;

void svc_time_format_time(char *buf, size_t len)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buf, len, "%H:%M", t);
}

void svc_time_format_date(char *buf, size_t len)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buf, len, "%Y/%m/%d", t);
}

void svc_time_format_weekday(char *buf, size_t len)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buf, len, "%A", t);
}

bool svc_time_is_synced(void)
{
    return s_synced;
}

static void time_sync_cb(struct timeval *tv)
{
    s_synced = true;
    ESP_LOGI(TAG, "NTP time synced");
}

static esp_err_t svc_time_on_init(svc_base_t *svc, void *ctx)
{
    (void)svc; (void)ctx;

    setenv("TZ", "CST-8", 1);
    tzset();

    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");
    config.sync_cb = time_sync_cb;
    config.start = true;
    esp_netif_sntp_init(&config);
    return ESP_OK;
}

static esp_err_t svc_time_on_start(svc_base_t *svc)
{
    (void)svc;
    return ESP_OK;
}

svc_base_t g_svc_time = {
    .name = "time",
    .deps = NULL,
    .dep_count = 0,
    .on_init = svc_time_on_init,
    .on_start = svc_time_on_start,
    .on_stop = NULL,
    .on_deinit = NULL,
};
