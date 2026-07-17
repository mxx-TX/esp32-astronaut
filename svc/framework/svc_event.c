#include "framework/svc_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <stdlib.h>

typedef struct svc_event_sub_s {
    svc_event_id_t id;
    svc_event_handler_t handler;
    void *ctx;
    struct svc_event_sub_s *next;
} svc_event_sub_t;

static svc_event_sub_t *s_subs = NULL;
static portMUX_TYPE s_lock = portMUX_INITIALIZER_UNLOCKED;
static QueueHandle_t s_async_q = NULL;
static int s_inited = 0;

static void async_task(void *arg)
{
    svc_event_id_t id;
    while (1) {
        if (xQueueReceive(s_async_q, &id, portMAX_DELAY) == pdPASS) {
            svc_event_publish(id, NULL);
        }
    }
}

static void ensure_init(void)
{
    if (s_inited) return;
    s_inited = 1;
    s_async_q = xQueueCreate(10, sizeof(svc_event_id_t));
    if (s_async_q) {
        xTaskCreate(async_task, "svc_evt", 3072, NULL, 3, NULL);
    }
}

esp_err_t svc_event_subscribe(svc_event_id_t id, svc_event_handler_t handler, void *ctx)
{
    ensure_init();
    svc_event_sub_t *sub = malloc(sizeof(*sub));
    if (!sub) { return ESP_ERR_NO_MEM; }
    sub->id = id; sub->handler = handler; sub->ctx = ctx;
    taskENTER_CRITICAL(&s_lock);
    sub->next = s_subs; s_subs = sub;
    taskEXIT_CRITICAL(&s_lock);
    return ESP_OK;
}

esp_err_t svc_event_publish(svc_event_id_t id, void *data)
{
    taskENTER_CRITICAL(&s_lock);
    svc_event_sub_t *s = s_subs;
    taskEXIT_CRITICAL(&s_lock);
    for (; s; s = s->next) {
        if (s->id == id) { s->handler(id, data, s->ctx); }
    }
    return ESP_OK;
}

esp_err_t svc_event_publish_async(svc_event_id_t id, void *data)
{
    (void)data;
    if (!s_async_q) { return ESP_ERR_INVALID_STATE; }
    return (xQueueSend(s_async_q, &id, 0) == pdPASS) ? ESP_OK : ESP_FAIL;
}

esp_err_t svc_event_unsubscribe(svc_event_id_t id, svc_event_handler_t handler)
{
    if (!s_inited) { return ESP_OK; }
    taskENTER_CRITICAL(&s_lock);
    svc_event_sub_t **pp = &s_subs;
    while (*pp) {
        if ((*pp)->id == id && (*pp)->handler == handler) {
            svc_event_sub_t *tmp = *pp;
            *pp = tmp->next; free(tmp);
            taskEXIT_CRITICAL(&s_lock);
            return ESP_OK;
        }
        pp = &(*pp)->next;
    }
    taskEXIT_CRITICAL(&s_lock);
    return ESP_ERR_NOT_FOUND;
}