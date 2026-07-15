#ifndef SVC_WIFI_H
#define SVC_WIFI_H
#include "framework/svc_manager.h"
#include "esp_err.h"
#include <stdbool.h>
typedef void (*svc_wifi_cb_t)(bool connected);
typedef void (*svc_wifi_prov_cb_t)(bool success);
esp_err_t svc_wifi_init(void);
esp_err_t svc_wifi_connect(const char *ssid, const char *pass, svc_wifi_cb_t cb);
esp_err_t svc_wifi_disconnect(void);
bool svc_wifi_is_connected(void);
esp_err_t svc_wifi_start_provisioning(svc_wifi_prov_cb_t cb);
esp_err_t svc_wifi_stop_provisioning(void);
bool svc_wifi_is_provisioning(void);

extern svc_base_t g_svc_wifi;

#endif
