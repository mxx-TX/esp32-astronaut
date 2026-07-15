#ifndef SVC_BLE_H
#define SVC_BLE_H
#include "esp_err.h"
#include <stdbool.h>

typedef void (*svc_ble_cred_cb_t)(const char *ssid, const char *pass);

esp_err_t svc_ble_init(svc_ble_cred_cb_t on_cred, const char *pin);
void svc_ble_deinit(void);
bool svc_ble_is_active(void);

#endif
