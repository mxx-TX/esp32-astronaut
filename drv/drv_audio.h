#ifndef DRV_AUDIO_H
#define DRV_AUDIO_H
#include "esp_err.h"
#include "driver/i2c_master.h"
#include "driver/i2s_std.h"
#include <stdint.h>

typedef struct {
    i2c_master_bus_handle_t i2c_bus;
    uint8_t dac_addr;
    uint8_t adc_addr;
    uint32_t i2c_speed;
    int i2s_port;
    int mclk;
    int bclk;
    int ws;
    int dout;
    int din;
    uint32_t sample_rate;
} drv_audio_cfg_t;

esp_err_t drv_audio_init(const drv_audio_cfg_t *cfg,
                         i2c_master_dev_handle_t *out_dac, i2c_master_dev_handle_t *out_adc,
                         i2s_chan_handle_t *out_tx, i2s_chan_handle_t *out_rx);
esp_err_t drv_audio_set_volume(i2c_master_dev_handle_t dac, int vol_percent);
esp_err_t drv_audio_set_mic_gain(i2c_master_dev_handle_t adc, int gain_db);

#endif
