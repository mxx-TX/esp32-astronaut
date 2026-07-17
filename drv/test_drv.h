#ifndef TEST_DRV_H
#define TEST_DRV_H
#include "esp_err.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_touch.h"

esp_err_t test_drv_run_all(esp_lcd_panel_handle_t lcd_panel,
                            esp_lcd_touch_handle_t touch_dev,
                            i2c_master_dev_handle_t dac_dev,
                            i2c_master_dev_handle_t adc_dev);

#endif