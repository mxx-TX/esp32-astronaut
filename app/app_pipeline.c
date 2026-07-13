#include "app_pipeline.h"
#include "app_fsm.h"
#include "svc_audio.h"
#include "svc_http.h"
#include "svc_storage.h"
#include "os_mem.h"
#include "esp_log.h"
#include <string.h>
static const char *TAG = "app_pipeline";
static i2s_chan_handle_t s_tx = NULL, s_rx = NULL;
static char s_asr_url[256], s_llm_url[256], s_tts_url[256], s_api_key[128];
esp_err_t app_pipeline_init(i2s_chan_handle_t tx, i2s_chan_handle_t rx)
{
    s_tx = tx; s_rx = rx;
    size_t len;
    len = sizeof(s_api_key); svc_storage_nvs_get_str("api_key", s_api_key, &len);
    len = sizeof(s_asr_url);  svc_storage_nvs_get_str("asr_url", s_asr_url, &len);
    len = sizeof(s_llm_url);  svc_storage_nvs_get_str("llm_url", s_llm_url, &len);
    len = sizeof(s_tts_url);  svc_storage_nvs_get_str("tts_url", s_tts_url, &len);
    return ESP_OK;
}
esp_err_t app_pipeline_start_wake_word(void) { return ESP_OK; }
esp_err_t app_pipeline_start_listen(void) { return ESP_OK; }
esp_err_t app_pipeline_send_audio(const uint8_t *data, size_t len)
{
    uint8_t resp[4096]; size_t rlen;
    esp_err_t ret = svc_http_post(s_asr_url, s_api_key, data, len, resp, &rlen, sizeof(resp));
    if (ret != ESP_OK) return ret;
    resp[rlen] = 0;
    ESP_LOGI(TAG, "ASR: %s", (char *)resp);
    app_fsm_post_event(APP_EVT_ASR_DONE);
    return ESP_OK;
}
