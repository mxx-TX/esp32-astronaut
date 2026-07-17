#include "svc_audio.h"
#include "freertos/FreeRTOS.h"
#include "bsp_audio.h"
#include "framework/svc_event.h"

static void pm_deep_sleep_handler(svc_event_id_t id, void *data, void *ctx)
{
    (void)id; (void)data; (void)ctx;
    /* I2S 会在深睡掉电后重新初始化，此处仅预留 */
}

esp_err_t svc_audio_play(i2s_chan_handle_t tx, const uint8_t *data, size_t len, uint32_t timeout_ms)
{
    size_t written = 0;
    return i2s_channel_write(tx, data, len, &written, pdMS_TO_TICKS(timeout_ms));
}
esp_err_t svc_audio_record(i2s_chan_handle_t rx, uint8_t *buf, size_t len, size_t *read, uint32_t timeout_ms)
{
    return i2s_channel_read(rx, buf, len, read, pdMS_TO_TICKS(timeout_ms));
}
esp_err_t svc_audio_set_vol(i2c_master_dev_handle_t dac, int pct)
{
    return bsp_audio_set_volume(dac, pct);
}

static esp_err_t svc_audio_on_init(svc_base_t *svc, void *ctx)
{
    (void)svc; (void)ctx;
    svc_event_subscribe(EVT_PM_DEEP_SLEEP, pm_deep_sleep_handler, NULL);
    return ESP_OK;
}

svc_base_t g_svc_audio = {
    .name = "audio",
    .deps = NULL,
    .dep_count = 0,
    .on_init = svc_audio_on_init,
    .on_start = NULL,
    .on_stop = NULL,
    .on_deinit = NULL,
};