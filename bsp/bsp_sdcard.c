#include "bsp_sdcard.h"
#include "driver/sdmmc_host.h"
#include "bsp_board_cfg.h"
#include "drv_sdcard.h"

esp_err_t bsp_sdcard_init(void)
{
    drv_sdcard_cfg_t cfg = {
        .slot = BSP_SD_SLOT,
        .clk  = BSP_SD_CLK,
        .cmd  = BSP_SD_CMD,
        .d0   = BSP_SD_D0,
        .mount_point = BSP_SD_MOUNT_POINT,
    };
    return drv_sdcard_init(&cfg);
}

esp_err_t bsp_sdcard_deinit(void)
{
    drv_sdcard_cfg_t cfg = { .mount_point = BSP_SD_MOUNT_POINT };
    return drv_sdcard_deinit(&cfg);
}
