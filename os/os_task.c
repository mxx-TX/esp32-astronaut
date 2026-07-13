#include "os_task.h"
esp_err_t os_task_create(void (*func)(void*), const char *name, uint32_t stack, void *arg, UBaseType_t prio, os_task_handle_t *handle, BaseType_t core)
{
    BaseType_t ret = xTaskCreatePinnedToCore(func, name, stack, arg, prio, handle, core);
    return (ret == pdPASS) ? ESP_OK : ESP_FAIL;
}
void os_task_delete(os_task_handle_t handle) { vTaskDelete(handle); }
void os_task_delay_ms(uint32_t ms) { vTaskDelay(pdMS_TO_TICKS(ms)); }
