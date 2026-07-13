#ifndef OS_MUTEX_H
#define OS_MUTEX_H
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_err.h"
typedef SemaphoreHandle_t os_mutex_handle_t;
esp_err_t os_mutex_create(os_mutex_handle_t *handle);
esp_err_t os_mutex_lock(os_mutex_handle_t handle, uint32_t timeout_ms);
esp_err_t os_mutex_unlock(os_mutex_handle_t handle);
#endif
