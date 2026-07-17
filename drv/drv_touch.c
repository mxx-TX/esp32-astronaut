#include "drv_touch.h"
#include "esp_lcd_touch_cst816s.h"
#include "esp_lcd_panel_io.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "drv_touch";

static void raw_i2c_probe(i2c_master_bus_handle_t bus, uint8_t addr, uint32_t speed)
{
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = addr,
        .scl_speed_hz = speed,
    };
    i2c_master_dev_handle_t h = NULL;
    if (i2c_master_bus_add_device(bus, &dev_cfg, &h) != ESP_OK) {
        ESP_LOGE(TAG, "can't create dev handle");
        return;
    }

    esp_err_t pr = i2c_master_probe(bus, addr, 500);
    ESP_LOGI(TAG, "probe 0x%02x: %s", addr, pr == ESP_OK ? "OK" : esp_err_to_name(pr));

    uint8_t reg = 0x00;  //触摸状态
    uint8_t val = 0;
    esp_err_t rr = i2c_master_transmit_receive(h, &reg, 1, &val, 1, 500);
    ESP_LOGI(TAG, "read reg0x00: %s (val=%d)", esp_err_to_name(rr), rr == ESP_OK ? val : -1);
    reg = 0xA7;  //芯片id
    rr = i2c_master_transmit_receive(h, &reg, 1, &val, 1, 500);
    ESP_LOGI(TAG, "read reg0xA7: %s (id=%d)", esp_err_to_name(rr), rr == ESP_OK ? val : -1);
    reg = 0x02;   //触摸点数及手势寄存器。第一个字节 0x00 表示触摸点数为0，后面的数据也因此无效
    uint8_t buf[5] = {0};
    rr = i2c_master_transmit_receive(h, &reg, 1, buf, 5, 500);
    ESP_LOGI(TAG, "read reg0x02(5B): %s", esp_err_to_name(rr));
    if (rr == ESP_OK) {
        ESP_LOGI(TAG, "  data: %02x %02x %02x %02x %02x", buf[0], buf[1], buf[2], buf[3], buf[4]);
    }
    i2c_master_bus_rm_device(h);
}

esp_err_t drv_touch_init(const drv_touch_cfg_t *cfg, esp_lcd_touch_handle_t *out_tp)
{
    raw_i2c_probe(cfg->i2c_bus, cfg->i2c_addr, cfg->i2c_speed);

    esp_lcd_panel_io_handle_t tp_io = NULL;
    esp_lcd_panel_io_i2c_config_t io_cfg = ESP_LCD_TOUCH_IO_I2C_CST816S_CONFIG();  //默认配置
    io_cfg.dev_addr = cfg->i2c_addr;
    io_cfg.scl_speed_hz = cfg->i2c_speed;
    esp_err_t ret = esp_lcd_new_panel_io_i2c(cfg->i2c_bus, &io_cfg, &tp_io);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "panel io fail: 0x%x", ret);
        return ret;
    }
    
    esp_lcd_touch_config_t tp_cfg = {
        .x_max = cfg->x_max,
        .y_max = cfg->y_max,
        .rst_gpio_num = cfg->rst_gpio,
        .int_gpio_num = cfg->int_gpio,
        .levels = { .reset = 0, .interrupt = 0 },
        .flags = { .swap_xy = 0, .mirror_x = 0, .mirror_y = 0 },
    };
    ret = esp_lcd_touch_new_i2c_cst816s(tp_io, &tp_cfg, out_tp);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "CST816S init fail: 0x%x", ret);
        esp_lcd_panel_io_del(tp_io);
        return ret;
    }
    ESP_LOGI(TAG, "CST816S touch init ok");
    return ESP_OK;
}

esp_err_t drv_touch_read(esp_lcd_touch_handle_t tp, drv_touch_data_t *data)
{
    if (!tp || !data) { return ESP_ERR_INVALID_ARG; }
    esp_err_t ret = esp_lcd_touch_read_data(tp);
    if (ret != ESP_OK) { return ret; }
    uint8_t cnt = 0;
    esp_lcd_touch_point_data_t pt;
    bool hit = esp_lcd_touch_get_data(tp, &pt, &cnt, 1);
    if (hit && cnt > 0) {
        data->pressed = true; data->x = pt.x; data->y = pt.y;
    } else {
        data->pressed = false; data->x = 0; data->y = 0;
    }
    return ESP_OK;
}

esp_err_t drv_touch_deinit(esp_lcd_touch_handle_t tp)
{
    if (!tp) { return ESP_ERR_INVALID_ARG; }
    esp_lcd_panel_io_handle_t io = tp->io;
    esp_err_t ret = esp_lcd_touch_del(tp);
    if (ret == ESP_OK && io) { esp_lcd_panel_io_del(io); }
    return ret;
}
