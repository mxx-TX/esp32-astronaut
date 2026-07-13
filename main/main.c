#include <stdio.h>
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bsp_board.h"
#include "svc_wifi.h"
#include "svc_display.h"
#include "svc_indicator.h"
#include "app_fsm.h"
#include "app_pipeline.h"
#include "test_os.h"
#include "test_bsp.h"
#include "test_svc.h"
static const char *TAG = "main";
static bsp_handles_t h;
void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }
    ESP_LOGI(TAG, "ESP-VoCat booting...");
    ESP_LOGI(TAG, "--- Layer Tests ---");
    test_os_run_all();
    ret = bsp_board_init(&h);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "BSP init fail: 0x%x", ret); return; }
    test_bsp_run_all(&h);
    test_svc_run_all(&h);
    ESP_LOGI(TAG, "--- App Init ---");
    ret = svc_display_init(h.lcd_panel, h.i2c_touch);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "Display init fail"); return; }
    svc_indicator_set(IND_SLOW);
    ESP_LOGI(TAG, "WiFi init...");
    svc_wifi_init();
    app_fsm_init();
    app_pipeline_init(h.i2s_tx, h.i2s_rx);
    ESP_LOGI(TAG, "Ready. Connect WiFi to start.");
    app_fsm_post_event(APP_EVT_BOOT_DONE);
    while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
}
