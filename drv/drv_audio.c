#include "drv_audio.h"
#include "esp_log.h"
#include <string.h>
#include "os_task.h"

static const char *TAG = "drv_audio";

/* 写 ES8311 寄存器 */
static esp_err_t es8311_write_reg(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};
    return i2c_master_transmit(dev, buf, 2, 100);
}

/* 读 ES8311 寄存器 */
static esp_err_t es8311_read_reg(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t *val)
{
    uint8_t cmd = reg;
    esp_err_t ret = i2c_master_transmit_receive(dev, &cmd, 1, val, 1, 100);
    if (ret != ESP_OK) {
        *val = 0;
    }
    return ret;
}

/* 写 ES7210 寄存器 */
static esp_err_t es7210_write_reg(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};
    return i2c_master_transmit(dev, buf, 2, 100);
}

/* 读 ES7210 寄存器 */
static esp_err_t es7210_read_reg(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t *val)
{
    uint8_t cmd = reg;
    esp_err_t ret = i2c_master_transmit_receive(dev, &cmd, 1, val, 1, 100);
    if (ret != ESP_OK) {
        *val = 0;
    }
    return ret;
}

/* ES7210 读-改-写 */
static esp_err_t es7210_update_reg(i2c_master_dev_handle_t dev, uint8_t reg, uint8_t mask, uint8_t val)
{
    uint8_t regv = 0;
    esp_err_t ret = es7210_read_reg(dev, reg, &regv);
    if (ret != ESP_OK) return ret;
    regv = (regv & ~mask) | (val & mask);
    return es7210_write_reg(dev, reg, regv);
}

/* 初始化 ES8311 DAC */
static esp_err_t es8311_init_codec(i2c_master_dev_handle_t dev)
{
    esp_err_t ret;
    uint8_t regv;

    /* === es8311_open 阶段：上电 + 基础配置 === */

    /* 读 REG0D，如果不是 0xFA 则上电 */
    ret = es8311_read_reg(dev, 0x0D, &regv);
    if (ret != ESP_OK) return ret;
    if (regv != 0xFA) {
        ret = es8311_write_reg(dev, 0x0D, 0xFA);
        if (ret != ESP_OK) return ret;
    }

    /* I2C 抗干扰增强（写两次防首次写入失败） */
    ret = es8311_write_reg(dev, 0x44, 0x08);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x44, 0x08);
    if (ret != ESP_OK) return ret;

    /* 时钟/系统寄存器初始值 */
    ret = es8311_write_reg(dev, 0x01, 0x30);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x02, 0x00);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x03, 0x10);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x16, 0x24);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x04, 0x10);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x05, 0x00);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x0B, 0x00);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x0C, 0x00);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x10, 0x1F);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x11, 0x7F);
    if (ret != ESP_OK) return ret;

    /* 数字复位 */
    ret = es8311_write_reg(dev, 0x00, 0x80);
    if (ret != ESP_OK) return ret;

    /* 设 Slave 模式（清除 bit6）*/
    ret = es8311_read_reg(dev, 0x00, &regv);
    if (ret != ESP_OK) return ret;
    regv &= ~0x40;
    ret = es8311_write_reg(dev, 0x00, regv);
    if (ret != ESP_OK) return ret;

    /* 时钟源：外部 MCLK，不翻转 */
    regv = 0x3F;
    regv &= ~0x80;  /* bit7=0: 用外部 MCLK */
    regv &= ~0x40;  /* bit6=0: 不翻转 MCLK */
    ret = es8311_write_reg(dev, 0x01, regv);
    if (ret != ESP_OK) return ret;

    /* BCLK 不翻转 */
    ret = es8311_read_reg(dev, 0x06, &regv);
    if (ret != ESP_OK) return ret;
    regv &= ~0x20;
    ret = es8311_write_reg(dev, 0x06, regv);
    if (ret != ESP_OK) return ret;

    ret = es8311_write_reg(dev, 0x13, 0x10);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x1B, 0x0A);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x1C, 0x6A);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x44, 0x08);
    if (ret != ESP_OK) return ret;

    /* === es8311_set_fs 阶段：I2S 格式 + 时钟分频（48kHz/16bit）=== */

    /* REG09/0A: I2S 格式（bits[1:0]=00），16bit（bits[3:2]=11） */
    ret = es8311_read_reg(dev, 0x09, &regv);
    if (ret != ESP_OK) return ret;
    regv = (regv & 0xE0) | 0x0C;
    ret = es8311_write_reg(dev, 0x09, regv);
    if (ret != ESP_OK) return ret;

    ret = es8311_read_reg(dev, 0x0A, &regv);
    if (ret != ESP_OK) return ret;
    regv = (regv & 0xE0) | 0x0C;
    ret = es8311_write_reg(dev, 0x0A, regv);
    if (ret != ESP_OK) return ret;

    /* 时钟分频（MCLK=12.288MHz, 48kHz 查 coeff_div 表） */
    ret = es8311_write_reg(dev, 0x02, 0x00);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x03, 0x10);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x04, 0x10);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x05, 0x00);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x06, 0x03);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x07, 0x00);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x08, 0xFF);
    if (ret != ESP_OK) return ret;

    /* === es8311_start 阶段：使能 DAC 输出 === */

    /* 数字复位 */
    ret = es8311_write_reg(dev, 0x00, 0x80);
    if (ret != ESP_OK) return ret;

    /* 时钟源 */
    ret = es8311_write_reg(dev, 0x01, 0x3F);
    if (ret != ESP_OK) return ret;

    /* SDP 清除 bit6（DAC/ADC 输出使能） */
    ret = es8311_read_reg(dev, 0x09, &regv);
    if (ret != ESP_OK) return ret;
    regv &= ~0x40;
    ret = es8311_write_reg(dev, 0x09, regv);
    if (ret != ESP_OK) return ret;

    ret = es8311_read_reg(dev, 0x0A, &regv);
    if (ret != ESP_OK) return ret;
    regv &= ~0x40;
    ret = es8311_write_reg(dev, 0x0A, regv);
    if (ret != ESP_OK) return ret;

    ret = es8311_write_reg(dev, 0x17, 0xBF);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x0E, 0x02);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x12, 0x00);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x14, 0x1A);
    if (ret != ESP_OK) return ret;

    /* 禁用数字 MIC（清除 bit6） */
    ret = es8311_read_reg(dev, 0x14, &regv);
    if (ret != ESP_OK) return ret;
    regv &= ~0x40;
    ret = es8311_write_reg(dev, 0x14, regv);
    if (ret != ESP_OK) return ret;

    ret = es8311_write_reg(dev, 0x0D, 0x01);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x15, 0x40);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x37, 0x08);
    if (ret != ESP_OK) return ret;
    ret = es8311_write_reg(dev, 0x45, 0x00);
    if (ret != ESP_OK) return ret;

    /* 取消静音 */
    ret = es8311_read_reg(dev, 0x31, &regv);
    if (ret != ESP_OK) return ret;
    regv &= ~0x60;
    ret = es8311_write_reg(dev, 0x31, regv);
    if (ret != ESP_OK) return ret;

    /* 初始音量 75% */
    ret = es8311_write_reg(dev, 0x32, 0xC0);
    if (ret != ESP_OK) return ret;

    ret = es8311_read_reg(dev, 0xFD, &regv);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ES8311 ID read FAILED - I2C not working?");
    } else {
        ESP_LOGI(TAG, "ES8311 chip ID: 0x%02X", regv);
    }

    // /* 回读关键寄存器验证 */
    // uint8_t regs[] = {0x00, 0x01, 0x09, 0x0A, 0x0D, 0x0E, 0x12, 0x14, 0x31, 0x32};
    // for (int i = 0; i < sizeof(regs); i++) {
    //     ret = es8311_read_reg(dev, regs[i], &regv);
    //     if (ret == ESP_OK) {
    //         ESP_LOGI(TAG, "  REG 0x%02X = 0x%02X", regs[i], regv);
    //     }
    // }
    ESP_LOGI(TAG, "ES8311 init done");
    return ESP_OK;

}

/* 初始化 ES7210 ADC */
static esp_err_t es7210_init_codec(i2c_master_dev_handle_t dev)
{
    esp_err_t ret;

    /* 复位 */
    ret = es7210_write_reg(dev, 0x00, 0xFF);
    if (ret != ESP_OK) return ret;
    ret = es7210_write_reg(dev, 0x00, 0x41);
    if (ret != ESP_OK) return ret;

    /* 关闭所有 ADC 时钟 */
    ret = es7210_write_reg(dev, 0x01, 0x3F);
    if (ret != ESP_OK) return ret;

    /* 芯片状态周期和上电周期 */
    ret = es7210_write_reg(dev, 0x09, 0x30);
    if (ret != ESP_OK) return ret;
    ret = es7210_write_reg(dev, 0x0A, 0x30);
    if (ret != ESP_OK) return ret;

    /* ADC 高通滤波 */
    ret = es7210_write_reg(dev, 0x23, 0x2A);
    if (ret != ESP_OK) return ret;
    ret = es7210_write_reg(dev, 0x22, 0x0A);
    if (ret != ESP_OK) return ret;
    ret = es7210_write_reg(dev, 0x20, 0x0A);
    if (ret != ESP_OK) return ret;
    ret = es7210_write_reg(dev, 0x21, 0x2A);
    if (ret != ESP_OK) return ret;

    /* I2S Slave 模式 */
    ret = es7210_update_reg(dev, 0x08, 0x01, 0x00);
    if (ret != ESP_OK) return ret;

    /* 模拟电源 3.3V，开启 VMID */
    ret = es7210_write_reg(dev, 0x40, 0x43);
    if (ret != ESP_OK) return ret;
    ret = es7210_write_reg(dev, 0x41, 0x70);
    if (ret != ESP_OK) return ret;
    ret = es7210_write_reg(dev, 0x42, 0x70);
    if (ret != ESP_OK) return ret;

    /* ADC OSR */
    ret = es7210_write_reg(dev, 0x07, 0x20);
    if (ret != ESP_OK) return ret;

    /* 主时钟分频：使用 DLL */
    ret = es7210_write_reg(dev, 0x02, 0xC1);
    if (ret != ESP_OK) return ret;

    /* 使能 MIC1+MIC2 时钟 */
    ret = es7210_update_reg(dev, 0x01, 0x0B, 0x00);
    if (ret != ESP_OK) return ret;

    /* MIC1+MIC2 电源开启 */
    ret = es7210_write_reg(dev, 0x4B, 0x00);
    if (ret != ESP_OK) return ret;
    ret = es7210_write_reg(dev, 0x47, 0x08);
    if (ret != ESP_OK) return ret;
    ret = es7210_write_reg(dev, 0x48, 0x08);
    if (ret != ESP_OK) return ret;

    /* MIC1 增益 30dB（bit4=1 使能模拟前端） */
    ret = es7210_write_reg(dev, 0x43, 0x1A);
    if (ret != ESP_OK) return ret;
    /* MIC2 增益 30dB */
    ret = es7210_write_reg(dev, 0x44, 0x1A);
    if (ret != ESP_OK) return ret;

    /* 关闭 MIC3+MIC4 */
    ret = es7210_write_reg(dev, 0x4C, 0xFF);
    if (ret != ESP_OK) return ret;

    /* 确认模拟电源 */
    ret = es7210_write_reg(dev, 0x40, 0x43);
    if (ret != ESP_OK) return ret;

    /* 退出复位进入工作 */
    ret = es7210_write_reg(dev, 0x00, 0x71);
    if (ret != ESP_OK) return ret;
    ret = es7210_write_reg(dev, 0x00, 0x41);
    if (ret != ESP_OK) return ret;

    /* 清 POWER_DOWN */
    ret = es7210_write_reg(dev, 0x06, 0x00);
    if (ret != ESP_OK) return ret;

    ESP_LOGI(TAG, "ES7210 init done");
    return ESP_OK;
}

esp_err_t drv_audio_init(const drv_audio_cfg_t *cfg,
                         i2c_master_dev_handle_t *out_dac, i2c_master_dev_handle_t *out_adc,
                         i2s_chan_handle_t *out_tx, i2s_chan_handle_t *out_rx)
{
    /* 挂载 DAC（ES8311）到 I2C 总线 */
    i2c_device_config_t dac_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = cfg->dac_addr,
        .scl_speed_hz = cfg->i2c_speed,
    };
    esp_err_t ret = i2c_master_bus_add_device(cfg->i2c_bus, &dac_cfg, out_dac);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "ES8311 add fail: 0x%x", ret); return ret; }

    /* 挂载 ADC（ES7210）到 I2C 总线 */
    i2c_device_config_t adc_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = cfg->adc_addr,
        .scl_speed_hz = cfg->i2c_speed,
    };
    ret = i2c_master_bus_add_device(cfg->i2c_bus, &adc_cfg, out_adc);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "ES7210 add fail: 0x%x", ret); return ret; }

    /* 初始化 DAC 和 ADC 寄存器 */
    ret = es8311_init_codec(*out_dac);
    if (ret != ESP_OK) return ret;
    ret = es7210_init_codec(*out_adc);
    if (ret != ESP_OK) return ret;

    /* 创建 I2S TX/RX 通道 */
    i2s_chan_config_t chan_cfg = {
        .id = cfg->i2s_port,
        .role = I2S_ROLE_MASTER,
        .dma_desc_num = 8,
        .dma_frame_num = 256,
        .auto_clear = true,
    };
    ret = i2s_new_channel(&chan_cfg, out_tx, out_rx);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "I2S chan fail: 0x%x", ret); return ret; }

    /* 配置 I2S 标准模式（48kHz，16bit，立体声） */
    i2s_std_config_t std_cfg = {
        .clk_cfg = {
            .sample_rate_hz = cfg->sample_rate,
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
            .mclk = cfg->mclk,
            .bclk = cfg->bclk,
            .ws = cfg->ws,
            .dout = cfg->dout,
            .din = cfg->din,
            .invert_flags = { .mclk_inv = false, .bclk_inv = false, .ws_inv = false },
        },
    };
    ret = i2s_channel_init_std_mode(*out_tx, &std_cfg);
    if (ret != ESP_OK) return ret;
    ret = i2s_channel_init_std_mode(*out_rx, &std_cfg);
    if (ret != ESP_OK) return ret;

    /* 使能 I2S 通道 */
    ret = i2s_channel_enable(*out_tx);
    if (ret != ESP_OK) return ret;
    ret = i2s_channel_enable(*out_rx);
    return ret;
}

/* 设置 DAC 音量 0-100% */
esp_err_t drv_audio_set_volume(i2c_master_dev_handle_t dac, int vol_percent)
{
    uint8_t vol = (vol_percent > 100) ? 100 : (uint8_t)vol_percent;
    uint8_t reg_val = (uint8_t)((uint32_t)vol * 255 / 100);
    return es8311_write_reg(dac, 0x32, reg_val);
}

/* 设置 ES7210 MIC 增益 */
esp_err_t drv_audio_set_mic_gain(i2c_master_dev_handle_t adc, int gain_db)
{
    uint8_t g;
    if (gain_db <= 0) {
        g = 0;
    } else if (gain_db >= 37) {
        g = 14;
    } else {
        g = (uint8_t)((gain_db + 1) / 3);
        if (g > 14) g = 14;
    }
    uint8_t val = 0x10 | g;
    esp_err_t ret = es7210_write_reg(adc, 0x43, val);
    if (ret != ESP_OK) return ret;
    ret = es7210_write_reg(adc, 0x44, val);
    if (ret != ESP_OK) return ret;
    ret = es7210_write_reg(adc, 0x45, val);
    if (ret != ESP_OK) return ret;
    return es7210_write_reg(adc, 0x46, val);
}
