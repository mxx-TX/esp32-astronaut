#include "os_event.h"
esp_err_t os_event_create(os_event_handle_t *handle)
{
    *handle = xEventGroupCreate();
    return (*handle) ? ESP_OK : ESP_FAIL;
}
esp_err_t os_event_set(os_event_handle_t handle, uint32_t bits)
{
    xEventGroupSetBits(handle, bits);
    return ESP_OK;
}
esp_err_t os_event_wait(os_event_handle_t handle, uint32_t bits, bool clear, bool wait_all, uint32_t timeout_ms)
{
    EventBits_t r = xEventGroupWaitBits(handle, bits, clear ? pdTRUE : pdFALSE, wait_all ? pdTRUE : pdFALSE, pdMS_TO_TICKS(timeout_ms));
    return ((r & bits) == bits) ? ESP_OK : ESP_FAIL;
}

