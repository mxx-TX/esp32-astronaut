#ifndef SVC_EVENT_H
#define SVC_EVENT_H
#include "esp_err.h"
#include <stdint.h>

typedef enum {
    EVT_WIFI_CONNECTED,
    EVT_WIFI_DISCONNECTED,
    EVT_NETWORK_UP,
    EVT_NETWORK_DOWN,
    EVT_WIFI_PROV_STARTED,
    EVT_WIFI_CONNECTING,
    EVT_USER = 100,
} svc_event_id_t;

typedef void (*svc_event_handler_t)(svc_event_id_t id, void *data, void *ctx);

esp_err_t svc_event_subscribe(svc_event_id_t id, svc_event_handler_t handler, void *ctx);
esp_err_t svc_event_publish(svc_event_id_t id, void *data);
esp_err_t svc_event_publish_async(svc_event_id_t id, void *data);
esp_err_t svc_event_unsubscribe(svc_event_id_t id, svc_event_handler_t handler);

#endif