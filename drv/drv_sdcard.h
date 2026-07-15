#ifndef DRV_SDCARD_H
#define DRV_SDCARD_H
#include "esp_err.h"
#include <stdint.h>

typedef struct {
    int slot;
    int clk;
    int cmd;
    int d0;
    const char *mount_point;
} drv_sdcard_cfg_t;

esp_err_t drv_sdcard_init(const drv_sdcard_cfg_t *cfg);
esp_err_t drv_sdcard_deinit(const drv_sdcard_cfg_t *cfg);

#endif
