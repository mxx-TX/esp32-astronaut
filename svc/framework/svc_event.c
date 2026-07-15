#include "framework/svc_event.h"
#include "os_mutex.h"
#include <stdlib.h>

typedef struct svc_event_sub_s {
    svc_event_id_t id;
    svc_event_handler_t handler;
    void *ctx;
    struct svc_event_sub_s *next;
} svc_event_sub_t;

static svc_event_sub_t *s_subs = NULL;
static os_mutex_handle_t s_mutex = NULL;

esp_err_t svc_event_subscribe(svc_event_id_t id, svc_event_handler_t handler, void *ctx)
{
    if (!s_mutex) { os_mutex_create(&s_mutex); }
    svc_event_sub_t *sub = malloc(sizeof(*sub));
    if (!sub) { return ESP_ERR_NO_MEM; }
    sub->id = id; sub->handler = handler; sub->ctx = ctx;
    os_mutex_lock(s_mutex, 1000);
    sub->next = s_subs; s_subs = sub;
    os_mutex_unlock(s_mutex);
    return ESP_OK;
}

esp_err_t svc_event_publish(svc_event_id_t id, void *data)
{
    if (!s_mutex) { os_mutex_create(&s_mutex); }
    os_mutex_lock(s_mutex, 1000);
    for (svc_event_sub_t *s = s_subs; s; s = s->next) {
        if (s->id == id) { s->handler(id, data, s->ctx); }
    }
    os_mutex_unlock(s_mutex);
    return ESP_OK;
}

esp_err_t svc_event_unsubscribe(svc_event_id_t id, svc_event_handler_t handler)
{
    if (!s_mutex) { return ESP_OK; }
    os_mutex_lock(s_mutex, 1000);
    svc_event_sub_t **pp = &s_subs;
    while (*pp) {
        if ((*pp)->id == id && (*pp)->handler == handler) {
            svc_event_sub_t *tmp = *pp;
            *pp = tmp->next; free(tmp);
            os_mutex_unlock(s_mutex);
            return ESP_OK;
        }
        pp = &(*pp)->next;
    }
    os_mutex_unlock(s_mutex);
    return ESP_ERR_NOT_FOUND;
}
