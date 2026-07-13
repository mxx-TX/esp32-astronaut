#include "bsp_touch.h"
#include "esp_log.h"
#include <string.h>
static const char *TAG = "bsp_touch";
esp_err_t bsp_touch_init(i2c_master_bus_handle_t i2c_bus, i2c_master_dev_handle_t *out_dev)
{
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x15,
        .scl_speed_hz = 400000,
    };
    esp_err_t ret = i2c_master_bus_add_device(i2c_bus, &dev_cfg, out_dev);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "add device failed: 0x%x", ret);
        return ret;
    }
    return ESP_OK;
}
esp_err_t bsp_touch_read(i2c_master_dev_handle_t dev, bsp_touch_data_t *data)
{
    uint8_t buf[6] = {0};
    uint8_t reg = 0x02;
    esp_err_t ret = i2c_master_transmit_receive(dev, &reg, 1, buf, 6, 100);
    if (ret != ESP_OK) {
        return ret;
    }
    data->x = ((uint16_t)(buf[1] & 0x0F) << 8) | buf[2];
    data->y = ((uint16_t)(buf[3] & 0x0F) << 8) | buf[4];
    data->pressed = (buf[0] > 0);
    return ESP_OK;
}
