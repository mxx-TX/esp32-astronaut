#include "os_mutex.h"
esp_err_t os_mutex_create(os_mutex_handle_t *handle)
{
    *handle = xSemaphoreCreateMutex();
    return (*handle) ? ESP_OK : ESP_FAIL;
}
esp_err_t os_mutex_lock(os_mutex_handle_t handle, uint32_t timeout_ms)
{
    return (xSemaphoreTake(handle, pdMS_TO_TICKS(timeout_ms)) == pdPASS) ? ESP_OK : ESP_FAIL;
}
esp_err_t os_mutex_unlock(os_mutex_handle_t handle)
{
    xSemaphoreGive(handle);
    return ESP_OK;
}
