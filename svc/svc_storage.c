#include "svc_storage.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
static nvs_handle_t s_nvs;
static bool s_nvs_ok = false;

esp_err_t svc_storage_nvs_set_str(const char *key, const char *val)
{
    if (!s_nvs_ok) { nvs_open("vocat", NVS_READWRITE, &s_nvs); s_nvs_ok = true; }
    return nvs_set_str(s_nvs, key, val);
}
esp_err_t svc_storage_nvs_get_str(const char *key, char *out, size_t *len)
{
    if (!s_nvs_ok) { nvs_open("vocat", NVS_READWRITE, &s_nvs); s_nvs_ok = true; }
    return nvs_get_str(s_nvs, key, out, len);
}
esp_err_t svc_storage_sd_write(const char *path, const uint8_t *data, size_t len)
{
    FILE *f = fopen(path, "wb");
    if (!f) return ESP_FAIL;
    fwrite(data, 1, len, f);
    fclose(f);
    return ESP_OK;
}
esp_err_t svc_storage_sd_read(const char *path, uint8_t *out, size_t *len, size_t max)
{
    FILE *f = fopen(path, "rb");
    if (!f) return ESP_FAIL;
    *len = fread(out, 1, max, f);
    fclose(f);
    return ESP_OK;
}
bool svc_storage_sd_exists(const char *path)
{
    struct stat st;
    return (stat(path, &st) == 0);
}

static esp_err_t svc_storage_on_init(svc_base_t *svc, void *ctx)
{
    (void)svc; (void)ctx;
    nvs_open("vocat", NVS_READWRITE, &s_nvs);
    s_nvs_ok = true;
    return ESP_OK;
}

svc_base_t g_svc_storage = {
    .name = "storage",
    .deps = NULL,
    .dep_count = 0,
    .on_init = svc_storage_on_init,
    .on_start = NULL,
    .on_stop = NULL,
    .on_deinit = NULL,
};
