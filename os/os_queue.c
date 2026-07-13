#include "os_queue.h"
esp_err_t os_queue_create(uint32_t item_size, uint32_t item_count, os_queue_handle_t *handle)
{
    *handle = xQueueCreate(item_count, item_size);
    return (*handle) ? ESP_OK : ESP_FAIL;
}
esp_err_t os_queue_send(os_queue_handle_t handle, const void *data, uint32_t timeout_ms)
{
    return (xQueueSend(handle, data, pdMS_TO_TICKS(timeout_ms)) == pdPASS) ? ESP_OK : ESP_FAIL;
}
esp_err_t os_queue_recv(os_queue_handle_t handle, void *data, uint32_t timeout_ms)
{
    return (xQueueReceive(handle, data, pdMS_TO_TICKS(timeout_ms)) == pdPASS) ? ESP_OK : ESP_FAIL;
}
