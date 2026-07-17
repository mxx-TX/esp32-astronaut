#include <stdio.h>
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp_board.h"
#include "framework/svc_manager.h"
#include "svc_storage.h"
#include "svc_indicator.h"
#include "svc_wifi.h"
#include "svc_http.h"
#include "svc_display.h"
#include "svc_audio.h"
#include "svc_time.h"
#include "svc_pm.h"
#include "svc_ota.h"
#include "app_fsm.h"
#include "app_pipeline.h"
#include "app_gui.h"
#include "test_bsp.h"
#include "test_svc.h"
#include "gui_wifi.h"

static const char *TAG = "main";
static bsp_handles_t h;

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    ESP_LOGI(TAG, "========== BSP Init ==========");
    ret = bsp_board_init(&h);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "BSP init fail: 0x%x", ret); return; }

    ESP_LOGI(TAG, "========== SVC Manager Init ==========");
    svc_manager_register(&g_svc_storage);
    svc_manager_register(&g_svc_indicator);
    svc_manager_register(&g_svc_wifi);
    svc_manager_register(&g_svc_http);
    svc_manager_register(&g_svc_time);
    svc_manager_register(&g_svc_display);
    svc_manager_register(&g_svc_pm);
    svc_manager_register(&g_svc_ota);
    svc_manager_register(&g_svc_audio);
    ret = svc_manager_init_all(&h);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "svc init fail: 0x%x", ret); return; }
    gui_wifi_init();
    app_gui_start();

    ret = svc_manager_start_all();
    if (ret != ESP_OK) { ESP_LOGE(TAG, "svc start fail: 0x%x", ret); return; }

    ESP_LOGI(TAG, "========== App Init ==========");
    app_fsm_init();
    app_pipeline_init(h.i2s_tx, h.i2s_rx);
    app_fsm_post_event(APP_EVT_BOOT_DONE);

    ESP_LOGI(TAG, "Ready.");

    while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
}