#ifndef OS_TASK_H
#define OS_TASK_H
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
typedef TaskHandle_t os_task_handle_t;
esp_err_t os_task_create(void (*func)(void*), const char *name, uint32_t stack, void *arg, UBaseType_t prio, os_task_handle_t *handle, BaseType_t core);
void os_task_delete(os_task_handle_t handle);
void os_task_delay_ms(uint32_t ms);
#endif
