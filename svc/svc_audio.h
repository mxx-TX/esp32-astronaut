#ifndef SVC_AUDIO_H
#define SVC_AUDIO_H
#include "framework/svc_manager.h"
#include "esp_err.h"
#include "driver/i2s_std.h"
#include "driver/i2c_master.h"
#include <stddef.h>
esp_err_t svc_audio_play(i2s_chan_handle_t tx, const uint8_t *data, size_t len, uint32_t timeout_ms);
esp_err_t svc_audio_record(i2s_chan_handle_t rx, uint8_t *buf, size_t len, size_t *read, uint32_t timeout_ms);
esp_err_t svc_audio_set_vol(i2c_master_dev_handle_t dac, int pct);

extern svc_base_t g_svc_audio;

#endif
