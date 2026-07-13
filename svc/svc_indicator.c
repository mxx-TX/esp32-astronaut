#include "svc_indicator.h"
#include "bsp_led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
static svc_ind_mode_t s_mode = IND_OFF;
static TaskHandle_t s_task = NULL;
static void ind_task(void *arg)
{
    while (1) {
        switch (s_mode) {
        case IND_OFF:    bsp_led_off(); vTaskDelay(pdMS_TO_TICKS(500)); break;
        case IND_ON:     bsp_led_on();  vTaskDelay(pdMS_TO_TICKS(500)); break;
        case IND_SLOW:   bsp_led_toggle(); vTaskDelay(pdMS_TO_TICKS(500)); break;
        case IND_FAST:   bsp_led_toggle(); vTaskDelay(pdMS_TO_TICKS(100)); break;
        case IND_BREATHE: bsp_led_on(); vTaskDelay(pdMS_TO_TICKS(300)); bsp_led_off(); vTaskDelay(pdMS_TO_TICKS(700)); break;
        }
    }
}
esp_err_t svc_indicator_set(svc_ind_mode_t mode)
{
    s_mode = mode;
    if (!s_task) xTaskCreate(ind_task, "ind", 2048, NULL, 1, &s_task);
    return ESP_OK;
}
