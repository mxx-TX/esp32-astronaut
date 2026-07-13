#ifndef BSP_SDCARD_H
#define BSP_SDCARD_H
#include "esp_err.h"

esp_err_t bsp_sdcard_init(void);
esp_err_t bsp_sdcard_deinit(void);

#endif
