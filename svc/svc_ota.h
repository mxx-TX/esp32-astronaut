#ifndef SVC_OTA_H
#define SVC_OTA_H
#include "framework/svc_manager.h"
#include "esp_err.h"

esp_err_t svc_ota_start(const char *url);
esp_err_t svc_ota_rollback(void);
extern svc_base_t g_svc_ota;

#endif