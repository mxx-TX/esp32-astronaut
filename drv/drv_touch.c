#include "drv_touch.h"
#include "esp_log.h"

static const char *TAG = "drv_touch";

esp_err_t drv_touch_init(const drv_touch_cfg_t *cfg, i2c_master_dev_handle_t *out_dev)
{
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = cfg->i2c_addr,
        .scl_speed_hz = cfg->i2c_speed,
    };
    esp_err_t ret = i2c_master_bus_add_device(cfg->i2c_bus, &dev_cfg, out_dev);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "add device failed: 0x%x", ret);
    }
    return ret;
}

esp_err_t drv_touch_read(i2c_master_dev_handle_t dev, drv_touch_data_t *data)
{
    if (!data) { return ESP_ERR_INVALID_ARG; }
    uint8_t buf[6] = {0};
    uint8_t reg = 0x02;
    esp_err_t ret = i2c_master_transmit_receive(dev, &reg, 1, buf, 6, 100);
    if (ret != ESP_OK) { return ret; }
    data->x = ((uint16_t)(buf[1] & 0x0F) << 8) | buf[2];
    data->y = ((uint16_t)(buf[3] & 0x0F) << 8) | buf[4];
    data->pressed = (buf[0] > 0);
    return ESP_OK;
}
