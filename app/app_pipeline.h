#ifndef APP_PIPELINE_H
#define APP_PIPELINE_H
#include "esp_err.h"
#include "driver/i2s_std.h"
esp_err_t app_pipeline_init(i2s_chan_handle_t tx, i2s_chan_handle_t rx);
esp_err_t app_pipeline_start_wake_word(void);
esp_err_t app_pipeline_start_listen(void);
esp_err_t app_pipeline_send_audio(const uint8_t *data, size_t len);
#endif
