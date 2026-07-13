#include "bsp_audio.h"
#include "esp_log.h"
#include <string.h>
static const char *TAG = "bsp_audio";
#define ES8311_ADDR 0x18
#define ES7210_ADDR 0x40
static esp_err_t es8311_write_reg(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};
    return i2c_master_transmit(dev, buf, 2, 100);
}
static esp_err_t es7210_write_reg(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};
    return i2c_master_transmit(dev, buf, 2, 100);
}
static esp_err_t es8311_init_codec(i2c_master_dev_handle_t dev)
{
    esp_err_t ret;
    ret = es8311_write_reg(dev, 0x00, 0x6F);
    if (ret) return ret;
    ret = es8311_write_reg(dev, 0x00, 0x00);
    ret = es8311_write_reg(dev, 0x01, 0x3F);
    ret = es8311_write_reg(dev, 0x02, 0x02);
    ret = es8311_write_reg(dev, 0x06, 0x00);
    ret = es8311_write_reg(dev, 0x08, 0x00);
    ret = es8311_write_reg(dev, 0x0B, 0x00);
    ret = es8311_write_reg(dev, 0x0C, 0x00);
    ret = es8311_write_reg(dev, 0x14, 0x00);
    ret = es8311_write_reg(dev, 0x15, 0x00);
    ret = es8311_write_reg(dev, 0x1A, 0x00);
    ret = es8311_write_reg(dev, 0x1B, 0x00);
    ret = es8311_write_reg(dev, 0x29, 0x00);
    ret = es8311_write_reg(dev, 0x2A, 0x00);
    ESP_LOGI(TAG, "ES8311 init done");
    return ret;
}
static esp_err_t es7210_init_codec(i2c_master_dev_handle_t dev)
{
    esp_err_t ret;
    ret = es7210_write_reg(dev, 0x00, 0xFF);
    if (ret) return ret;
    ret = es7210_write_reg(dev, 0x00, 0x41);
    ret = es7210_write_reg(dev, 0x01, 0x1F);
    ret = es7210_write_reg(dev, 0x06, 0x00);
    ret = es7210_write_reg(dev, 0x07, 0x20);
    ret = es7210_write_reg(dev, 0x08, 0x01);
    ret = es7210_write_reg(dev, 0x09, 0x02);
    ret = es7210_write_reg(dev, 0x0A, 0x00);
    ret = es7210_write_reg(dev, 0x40, 0x0E);
    ret = es7210_write_reg(dev, 0x41, 0x0A);
    ret = es7210_write_reg(dev, 0x42, 0x0A);
    ret = es7210_write_reg(dev, 0x43, 0x0A);
    ret = es7210_write_reg(dev, 0x44, 0x0A);
    ESP_LOGI(TAG, "ES7210 init done");
    return ret;
}
esp_err_t bsp_audio_init(i2c_master_bus_handle_t i2c_bus,
                         i2c_master_dev_handle_t *out_dac, i2c_master_dev_handle_t *out_adc,
                         i2s_chan_handle_t *out_tx, i2s_chan_handle_t *out_rx)
{
    i2c_device_config_t dac_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = ES8311_ADDR,
        .scl_speed_hz = 400000,
    };
    esp_err_t ret = i2c_master_bus_add_device(i2c_bus, &dac_cfg, out_dac);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "ES8311 add fail: 0x%x", ret); return ret; }

    i2c_device_config_t adc_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = ES7210_ADDR,
        .scl_speed_hz = 400000,
    };
    ret = i2c_master_bus_add_device(i2c_bus, &adc_cfg, out_adc);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "ES7210 add fail: 0x%x", ret); return ret; }

    ret = es8311_init_codec(*out_dac);
    if (ret) return ret;
    ret = es7210_init_codec(*out_adc);
    if (ret) return ret;

    i2s_chan_config_t chan_cfg = {
        .id = I2S_NUM_0,
        .role = I2S_ROLE_MASTER,
        .dma_desc_num = 8,
        .dma_frame_num = 256,
        .auto_clear = true,
    };
    ret = i2s_new_channel(&chan_cfg, out_tx, out_rx);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "I2S chan fail: 0x%x", ret); return ret; }

    i2s_std_config_t std_cfg = {
        .clk_cfg = {
            .sample_rate_hz = 48000,
            .clk_src = I2S_CLK_SRC_DEFAULT,
            .mclk_multiple = I2S_MCLK_MULTIPLE_256,
        },
        .slot_cfg = {
            .data_bit_width = I2S_DATA_BIT_WIDTH_16BIT,
            .slot_bit_width = I2S_SLOT_BIT_WIDTH_AUTO,
            .slot_mode = I2S_SLOT_MODE_STEREO,
            .slot_mask = I2S_STD_SLOT_BOTH,
            .ws_width = 16,
            .ws_pol = false,
            .bit_shift = true,
        },
        .gpio_cfg = {
            .mclk = 42,
            .bclk = 40,
            .ws = 39,
            .dout = 41,
            .din = 3,
            .invert_flags = { .mclk_inv = false, .bclk_inv = false, .ws_inv = false },
        },
    };
    ret = i2s_channel_init_std_mode(*out_tx, &std_cfg);
    if (ret) return ret;
    ret = i2s_channel_init_std_mode(*out_rx, &std_cfg);
    if (ret) return ret;
    ret = i2s_channel_enable(*out_tx);
    if (ret) return ret;
    ret = i2s_channel_enable(*out_rx);
    return ret;
}
esp_err_t bsp_audio_set_volume(i2c_master_dev_handle_t dac, int vol_percent)
{
    uint8_t vol = (uint8_t)((vol_percent * 100) / 100);
    if (vol > 100) vol = 100;
    esp_err_t ret = es8311_write_reg(dac, 0x14, vol);
    if (ret) return ret;
    return es8311_write_reg(dac, 0x15, vol);
}
esp_err_t bsp_audio_set_mic_gain(i2c_master_dev_handle_t adc, int gain_db)
{
    uint8_t g = (uint8_t)gain_db;
    esp_err_t ret = es7210_write_reg(adc, 0x41, g);
    if (ret) return ret;
    ret = es7210_write_reg(adc, 0x42, g);
    if (ret) return ret;
    ret = es7210_write_reg(adc, 0x43, g);
    if (ret) return ret;
    return es7210_write_reg(adc, 0x44, g);
}
