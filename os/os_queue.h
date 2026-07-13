#ifndef OS_QUEUE_H
#define OS_QUEUE_H
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_err.h"
typedef QueueHandle_t os_queue_handle_t;
esp_err_t os_queue_create(uint32_t item_size, uint32_t item_count, os_queue_handle_t *handle);
esp_err_t os_queue_send(os_queue_handle_t handle, const void *data, uint32_t timeout_ms);
esp_err_t os_queue_recv(os_queue_handle_t handle, void *data, uint32_t timeout_ms);
#endif
