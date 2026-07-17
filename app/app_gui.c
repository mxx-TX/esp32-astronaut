#include "app_gui.h"
#include "app_fsm.h"
#include "ui.h"
#include "lvgl.h"
#include "esp_lvgl_port.h"
#include "esp_log.h"
#include "svc_time.h"
#include "svc_wifi.h"
#include "gui_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "app_gui";

static void time_update_task(void *arg)
{
    char buf[64];
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(5000));
        if (!svc_time_is_synced()) continue;
        if (!ui_ScreenHome) continue;
        lvgl_port_lock(0);
        svc_time_format_time(buf, sizeof(buf));
        lv_label_set_text(ui_HomeLabelTime, buf);
        svc_time_format_date(buf, sizeof(buf));
        lv_label_set_text(ui_HomeLabelData, buf);
        svc_time_format_weekday(buf, sizeof(buf));
        lv_label_set_text(ui_HomeLabelweekday, buf);
        lvgl_port_unlock();
    }
}

void app_gui_start(void)
{
    lvgl_port_lock(0);
    ui_init();
    lvgl_port_unlock();

    if (svc_wifi_is_provisioning()) {
        char buf[128];
        snprintf(buf, sizeof(buf), "Provisioning mode\nConnect to hotspot Astronaut_Setup\nPIN: %s",
                 svc_wifi_get_prov_pin());
        gui_wifi_show_message(buf);
    }

    xTaskCreate(time_update_task, "time_ui", 4096, NULL, 3, NULL);

    ESP_LOGI(TAG, "GUI started");
}

void app_gui_update_state(app_state_t state)
{
    (void)state;
}