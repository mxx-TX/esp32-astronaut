idf_component_register(SRCS "bsp_sdcard.c" "bsp_board.c" "bsp_lcd.c" "bsp_touch.c" "bsp_audio.c" "bsp_led.c" "test_bsp.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_driver_spi esp_driver_i2c esp_driver_i2s esp_driver_gpio esp_driver_ledc esp_driver_sdmmc esp_lcd fatfs esp_lcd_st77916 drv)
