#ifndef BSP_AUDIO_H
#define BSP_AUDIO_H
#include "esp_err.h"
#include "driver/i2c_master.h"
#include "driver/i2s_std.h"

esp_err_t bsp_audio_init(i2c_master_bus_handle_t i2c_bus,
                         i2c_master_dev_handle_t *out_dac, i2c_master_dev_handle_t *out_adc,
                         i2s_chan_handle_t *out_tx, i2s_chan_handle_t *out_rx);
esp_err_t bsp_audio_set_volume(i2c_master_dev_handle_t dac, int vol_percent);
esp_err_t bsp_audio_set_mic_gain(i2c_master_dev_handle_t adc, int gain_db);

#endif
