#ifndef SVC_WIFI_H
#define SVC_WIFI_H
#include "esp_err.h"
#include <stdbool.h>
typedef void (*svc_wifi_cb_t)(bool connected);
esp_err_t svc_wifi_init(void);
esp_err_t svc_wifi_connect(const char *ssid, const char *pass, svc_wifi_cb_t cb);
esp_err_t svc_wifi_disconnect(void);
bool svc_wifi_is_connected(void);
#endif
