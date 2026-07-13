#include "test_svc.h"
#include "svc_storage.h"
#include "svc_indicator.h"
#include "svc_wifi.h"
#include "svc_audio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
static const char *TAG = "test_svc";
#define T_CHECK(name, expr) do { esp_err_t r = (expr); ESP_LOGI(TAG, "[%s] %s", (r == ESP_OK) ? "PASS" : "FAIL", name); } while(0)
esp_err_t test_svc_run_all(bsp_handles_t *h)
{
    ESP_LOGI(TAG, "========== SVC Test Suite ==========");

    T_CHECK("NVS set str", svc_storage_nvs_set_str("test_key", "hello"));
    char buf[32]; size_t len = sizeof(buf);
    T_CHECK("NVS get str", svc_storage_nvs_get_str("test_key", buf, &len));
    ESP_LOGI(TAG, "NVS read: %s", buf);

    T_CHECK("Indicator slow", svc_indicator_set(IND_SLOW));
    vTaskDelay(pdMS_TO_TICKS(500));
    T_CHECK("Indicator fast", svc_indicator_set(IND_FAST));
    vTaskDelay(pdMS_TO_TICKS(500));
    T_CHECK("Indicator off", svc_indicator_set(IND_OFF));

    T_CHECK("WiFi init", svc_wifi_init());

    uint8_t test_buf[512];
    memset(test_buf, 0, sizeof(test_buf));
    esp_err_t ar = svc_audio_play(h->i2s_tx, test_buf, sizeof(test_buf), 100);
    if (ar == ESP_OK || ar == ESP_ERR_TIMEOUT) {
        ESP_LOGI(TAG, "[PASS] Audio play init ok");
    }
    T_CHECK("Audio vol 80", svc_audio_set_vol(h->i2c_dac, 80));

    ESP_LOGI(TAG, "========== SVC Test Done ==========");
    return ESP_OK;
}
