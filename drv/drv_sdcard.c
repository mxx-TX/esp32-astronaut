#include "drv_sdcard.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "esp_log.h"

static const char *TAG = "drv_sdcard";

esp_err_t drv_sdcard_init(const drv_sdcard_cfg_t *cfg)
{
    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    host.slot = cfg->slot;
    host.max_freq_khz = SDMMC_FREQ_DEFAULT;

    sdmmc_slot_config_t slot_cfg = SDMMC_SLOT_CONFIG_DEFAULT();
    slot_cfg.clk = cfg->clk;
    slot_cfg.cmd = cfg->cmd;
    slot_cfg.d0 = cfg->d0;
    slot_cfg.width = 1;
    slot_cfg.flags = SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

    esp_vfs_fat_sdmmc_mount_config_t mount_cfg = {
        .format_if_mount_failed = false,
        .max_files = 5,
        .allocation_unit_size = 16 * 1024,
    };
    sdmmc_card_t *card = NULL;
    esp_err_t ret = esp_vfs_fat_sdmmc_mount(cfg->mount_point, &host, &slot_cfg, &mount_cfg, &card);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SD mount failed: %s", esp_err_to_name(ret));
        return ret;
    }
    sdmmc_card_print_info(stdout, card);
    return ESP_OK;
}

esp_err_t drv_sdcard_deinit(const drv_sdcard_cfg_t *cfg)
{
    return esp_vfs_fat_sdcard_unmount(cfg->mount_point, NULL);
}
