#include "bsp_audio.h"
#include "bsp_board_cfg.h"
#include "drv_audio.h"

esp_err_t bsp_audio_init(i2c_master_bus_handle_t i2c_bus,
                         i2c_master_dev_handle_t *out_dac, i2c_master_dev_handle_t *out_adc,
                         i2s_chan_handle_t *out_tx, i2s_chan_handle_t *out_rx)
{
    drv_audio_cfg_t cfg = {
        .i2c_bus     = i2c_bus,
        .dac_addr    = BSP_AUDIO_DAC_ADDR,
        .adc_addr    = BSP_AUDIO_ADC_ADDR,
        .i2c_speed   = 400000,
        .i2s_port    = BSP_AUDIO_I2S_NUM,
        .mclk        = BSP_AUDIO_MCLK,
        .bclk        = BSP_AUDIO_BCLK,
        .ws          = BSP_AUDIO_WS,
        .dout        = BSP_AUDIO_DOUT,
        .din         = BSP_AUDIO_DIN,
        .sample_rate = BSP_AUDIO_SAMPLE_RATE,
    };
    return drv_audio_init(&cfg, out_dac, out_adc, out_tx, out_rx);
}

esp_err_t bsp_audio_set_volume(i2c_master_dev_handle_t dac, int vol_percent)
{
    return drv_audio_set_volume(dac, vol_percent);
}

esp_err_t bsp_audio_set_mic_gain(i2c_master_dev_handle_t adc, int gain_db)
{
    return drv_audio_set_mic_gain(adc, gain_db);
}
