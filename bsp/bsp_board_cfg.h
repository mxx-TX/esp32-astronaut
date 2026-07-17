#ifndef BSP_BOARD_CFG_H
#define BSP_BOARD_CFG_H

#define BSP_LCD_SPI_HOST        SPI2_HOST
#define BSP_LCD_MOSI            46
#define BSP_LCD_MISO            13
#define BSP_LCD_SCLK            18
#define BSP_LCD_QWP             11
#define BSP_LCD_QHD             12
#define BSP_LCD_CS              14
#define BSP_LCD_DC              45
#define BSP_LCD_RST             3
#define BSP_LCD_BL              44
#define BSP_LCD_WIDTH           360
#define BSP_LCD_HEIGHT          360

//0x15 0x2A
#define BSP_TOUCH_I2C_ADDR      0x15   
#define BSP_TOUCH_I2C_SPEED     100000
#define BSP_TOUCH_INT_GPIO      10
#define BSP_TOUCH_RST_GPIO      -1

#define BSP_AUDIO_DAC_ADDR      0x18
#define BSP_AUDIO_ADC_ADDR      0x40
#define BSP_AUDIO_I2S_NUM       I2S_NUM_0
#define BSP_AUDIO_MCLK          42
#define BSP_AUDIO_BCLK          40
#define BSP_AUDIO_WS            39
#define BSP_AUDIO_DOUT          41
#define BSP_AUDIO_DIN           15
#define BSP_AUDIO_SAMPLE_RATE   48000

#define BSP_LED_GPIO            43

#define BSP_SD_SLOT             SDMMC_HOST_SLOT_1
#define BSP_SD_CLK              16
#define BSP_SD_CMD              38
#define BSP_SD_D0               17
#define BSP_SD_MOUNT_POINT      "/sdcard"

#define BSP_GPIO_LCD_EN         9
#define BSP_GPIO_PA_EN          4

#define BSP_I2C_PORT            I2C_NUM_0
#define BSP_I2C_SDA             2
#define BSP_I2C_SCL             1

#endif
