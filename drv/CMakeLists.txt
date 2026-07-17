idf_component_register(SRCS "test_drv.c" "drv_lcd.c" "drv_touch.c" "drv_audio.c" "drv_led.c" "drv_sdcard.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_driver_spi esp_driver_i2c esp_driver_i2s esp_driver_gpio esp_driver_ledc esp_driver_sdmmc esp_lcd fatfs esp_lcd_st77916 esp_lcd_touch esp_lcd_touch_cst816s os)