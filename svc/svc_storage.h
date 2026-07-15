#ifndef SVC_STORAGE_H
#define SVC_STORAGE_H
#include "framework/svc_manager.h"
#include "esp_err.h"
#include <stddef.h>
#include <stdbool.h>
esp_err_t svc_storage_nvs_set_str(const char *key, const char *val);
esp_err_t svc_storage_nvs_get_str(const char *key, char *out, size_t *len);
esp_err_t svc_storage_sd_write(const char *path, const uint8_t *data, size_t len);
esp_err_t svc_storage_sd_read(const char *path, uint8_t *out, size_t *len, size_t max);
bool svc_storage_sd_exists(const char *path);

extern svc_base_t g_svc_storage;

#endif
