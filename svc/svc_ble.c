/* ================================================================
 *  BLE 配网 - NimBLE GATT 服务器
 *  通过 UUID 0xFF00 暴露 WiFi 凭据服务，
 *  供 ESP BLE 配网 APP 或自定义 BLE 客户端使用。
 * ================================================================ */

#include "svc_ble.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include <string.h>
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

static const char *TAG = "svc_ble";



#define BLE_NAME  "Astronaut_Prov"

#define BLE_SVC_UUID   0xFF00
#define BLE_CHR_SSID   0xFF01
#define BLE_CHR_PASS   0xFF02
#define BLE_CHR_STATUS 0xFF03
#define BLE_CHR_PIN    0xFF04



static bool s_active = false;
static svc_ble_cred_cb_t s_on_cred = NULL;
static const char *s_pin = NULL;

static uint8_t s_ssid[33];
static uint8_t s_pass[65];
static bool s_ssid_set = false;
static bool s_pass_set = false;
static uint8_t s_status;

static uint16_t s_status_handle;
static uint16_t s_ssid_handle;
static uint16_t s_pass_handle;
static uint16_t s_pin_handle;

static uint16_t s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
static uint8_t  own_addr_type;

/* ================================================================
 *  GATT 访问回调
 * ================================================================ */

static int ble_gatt_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                              struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    (void)conn_handle; (void)arg;

    switch (ctxt->op) {

    case BLE_GATT_ACCESS_OP_READ_CHR:
        if (attr_handle == s_pin_handle) {
            os_mbuf_append(ctxt->om, s_pin, strlen(s_pin));
            return 0;
        }
        if (attr_handle == s_status_handle) {
            os_mbuf_append(ctxt->om, &s_status, 1);
            return 0;
        }
        return BLE_ATT_ERR_UNLIKELY;

    case BLE_GATT_ACCESS_OP_WRITE_CHR:
        if (attr_handle == s_ssid_handle) {
            uint16_t om_len = OS_MBUF_PKTLEN(ctxt->om);
            if (om_len > 32) return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            ble_hs_mbuf_to_flat(ctxt->om, s_ssid, om_len, NULL);
            s_ssid[om_len] = 0;
            s_ssid_set = true;
            ESP_LOGI(TAG, "SSID: %s", s_ssid);
        } else if (attr_handle == s_pass_handle) {
            uint16_t om_len = OS_MBUF_PKTLEN(ctxt->om);
            if (om_len > 64) return BLE_ATT_ERR_INVALID_ATTR_VALUE_LEN;
            ble_hs_mbuf_to_flat(ctxt->om, s_pass, om_len, NULL);
            s_pass[om_len] = 0;
            s_pass_set = true;
            ESP_LOGI(TAG, "password received");
        } else {
            return BLE_ATT_ERR_UNLIKELY;
        }

        if (s_ssid_set && s_pass_set && s_on_cred) {
            s_status = 2;
            ble_gatts_chr_updated(s_status_handle);
            s_on_cred((const char *)s_ssid, (const char *)s_pass);
        }
        return 0;

    default:
        return BLE_ATT_ERR_UNLIKELY;
    }
}

/* ================================================================
 *  GATT 服务定义
 * ================================================================ */

static const struct ble_gatt_svc_def gatt_svcs[] = {
    { .type = BLE_GATT_SVC_TYPE_PRIMARY,
      .uuid = BLE_UUID16_DECLARE(BLE_SVC_UUID),
      .characteristics = (struct ble_gatt_chr_def[]) {
        { .uuid = BLE_UUID16_DECLARE(BLE_CHR_SSID),
          .access_cb = ble_gatt_access_cb,
          .flags = BLE_GATT_CHR_F_WRITE,
          .val_handle = &s_ssid_handle },
        { .uuid = BLE_UUID16_DECLARE(BLE_CHR_PASS),
          .access_cb = ble_gatt_access_cb,
          .flags = BLE_GATT_CHR_F_WRITE,
          .val_handle = &s_pass_handle },
        { .uuid = BLE_UUID16_DECLARE(BLE_CHR_STATUS),
          .access_cb = ble_gatt_access_cb,
          .flags = BLE_GATT_CHR_F_READ | BLE_GATT_CHR_F_NOTIFY,
          .val_handle = &s_status_handle },
        { .uuid = BLE_UUID16_DECLARE(BLE_CHR_PIN),
          .access_cb = ble_gatt_access_cb,
          .flags = BLE_GATT_CHR_F_READ,
          .val_handle = &s_pin_handle },
        { 0 } } },
    { 0 }
};

/* ================================================================
 *  GAP 事件处理
 * ================================================================ */

static int ble_gap_event_handler(struct ble_gap_event *event, void *arg)
{
    (void)arg;

    switch (event->type) {

    case BLE_GAP_EVENT_CONNECT:
        if (event->connect.status == 0) {
            s_conn_handle = event->connect.conn_handle;
        } else if (s_active) {
            ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                              NULL, ble_gap_event_handler, NULL);
        }
        return 0;

    case BLE_GAP_EVENT_DISCONNECT:
        s_conn_handle = BLE_HS_CONN_HANDLE_NONE;
        s_ssid_set = false;
        s_pass_set = false;
        if (s_active) {
            ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                              NULL, ble_gap_event_handler, NULL);
        }
        return 0;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        if (s_active) {
            ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER,
                              NULL, ble_gap_event_handler, NULL);
        }
        return 0;

    default:
        return 0;
    }
}

/* ================================================================
 *  广播
 * ================================================================ */

static void ble_start_advertising(void)
{
    struct ble_gap_adv_params adv = {
        .conn_mode = BLE_GAP_CONN_MODE_UND,
        .disc_mode = BLE_GAP_DISC_MODE_GEN
    };
    struct ble_hs_adv_fields fields = {
        .flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP,
        .name = (uint8_t *)BLE_NAME,
        .name_len = strlen(BLE_NAME),
        .name_is_complete = 1
    };
    ble_gap_adv_set_fields(&fields);
    ble_gap_adv_start(own_addr_type, NULL, BLE_HS_FOREVER, &adv,
                      ble_gap_event_handler, NULL);
    ESP_LOGI(TAG, "advertising as '%s'", BLE_NAME);
}

/* ================================================================
 *  NimBLE 主机任务
 * ================================================================ */

static void ble_host_task(void *param)
{
    (void)param;
    nimble_port_run();
    nimble_port_freertos_deinit();
}

/* ================================================================
 *  NimBLE 同步回调
 * ================================================================ */

static void ble_on_sync(void)
{
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_gatts_count_cfg(gatt_svcs);
    ble_gatts_add_svcs(gatt_svcs);
    ble_start_advertising();
}

/* ================================================================
 *  公开 API
 * ================================================================ */

esp_err_t svc_ble_init(svc_ble_cred_cb_t on_cred, const char *pin)
{
    if (s_active) return ESP_ERR_INVALID_STATE;

    s_active = true;
    s_on_cred = on_cred;
    s_pin = pin;
    s_ssid_set = false;
    s_pass_set = false;
    s_status = 0;
    s_conn_handle = BLE_HS_CONN_HANDLE_NONE;

    nimble_port_init();
    ble_hs_cfg.sync_cb = ble_on_sync;
    nimble_port_freertos_init(ble_host_task);

    ESP_LOGI(TAG, "BLE provisioning started");
    return ESP_OK;
}

void svc_ble_deinit(void)
{
    if (!s_active) return;
    s_active = false;

    if (s_conn_handle != BLE_HS_CONN_HANDLE_NONE) {
        ble_gap_terminate(s_conn_handle, BLE_ERR_REM_USER_CONN_TERM);
    }

    int r = nimble_port_stop();
    if (r == 0) nimble_port_deinit();

    s_on_cred = NULL;
    s_pin = NULL;
    ESP_LOGI(TAG, "BLE provisioning stopped");
}

bool svc_ble_is_active(void)
{
    return s_active;
}
