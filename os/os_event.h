#ifndef OS_EVENT_H
#define OS_EVENT_H
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_err.h"
typedef EventGroupHandle_t os_event_handle_t;
#define OS_EVENT_BIT(n) (1ULL << (n))
esp_err_t os_event_create(os_event_handle_t *handle);
esp_err_t os_event_set(os_event_handle_t handle, uint32_t bits);
esp_err_t os_event_wait(os_event_handle_t handle, uint32_t bits, bool clear, bool wait_all, uint32_t timeout_ms);
#endif
