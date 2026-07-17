/* ================================================================
 *  WiFi 服务 - STA 连接、NVS 凭据存取、配网
 *  (Soft-AP HTTP 强制门户 + DNS 重定向)
 *  BLE 配网: 见 svc_ble.c
 * ================================================================ */

#include "svc_wifi.h"
#include "framework/svc_event.h"
#include "svc_storage.h"
#include "svc_ble.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_http_server.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <lwip/netdb.h>

static const char *TAG = "svc_wifi";


#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

#define AP_SSID  "Astronaut_Setup"
#define AP_IP    "192.168.4.1"

static bool s_connected = false;
static svc_wifi_cb_t s_cb = NULL;
static EventGroupHandle_t s_evt;

static bool s_provisioning = false;
static svc_wifi_prov_cb_t s_prov_cb = NULL;
static httpd_handle_t s_httpd = NULL;
static int s_dns_sock = -1;
static char s_prov_pin[8];

/* ================================================================
 *  NVS 凭据存取
 * ================================================================ */

static void save_creds_to_nvs(const char *ssid, const char *pass)
{
    svc_storage_nvs_set_str("wifi_ssid", ssid);
    svc_storage_nvs_set_str("wifi_pass", pass);
}

static bool load_creds_from_nvs(char *ssid, size_t ssid_len,
                                char *pass, size_t pass_len)
{
    size_t len = ssid_len;
    if (svc_storage_nvs_get_str("wifi_ssid", ssid, &len) != ESP_OK) return false;
    if (strlen(ssid) == 0) return false;
    len = pass_len;
    if (svc_storage_nvs_get_str("wifi_pass", pass, &len) != ESP_OK) return false;
    return true;
}

/* ================================================================
 *  PIN 生成
 * ================================================================ */

static void generate_pin(void)
{
    snprintf(s_prov_pin, sizeof(s_prov_pin), "%06d",
             (int)(esp_random() % 1000000));
    ESP_LOGI(TAG, "Provisioning PIN: %s", s_prov_pin);
}

/* ================================================================
 *  HTTP 处理器 - GET /  (配网首页)
 * ================================================================ */

static esp_err_t http_get_root_handler(httpd_req_t *req)
{
    char html[768];
    snprintf(html, sizeof(html),
        "<!DOCTYPE html><html><head><meta charset=\"UTF-8\">"
        "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">"
        "<title>WiFi Setup</title></head>"
        "<body style=\"font-family:sans-serif;padding:20px\">"
        "<h2>WiFi Setup</h2><p>PIN: <b>%s</b></p>"
        "<form action=\"/connect\" method=\"post\">"
        "<label>SSID:<br><input name=\"ssid\" maxlength=\"32\"></label><br><br>"
        "<label>Password:<br><input name=\"pass\" type=\"password\" maxlength=\"64\">"
        "</label><br><br>"
        "<label>PIN:<br><input name=\"pin\" maxlength=\"6\"></label><br><br>"
        "<button type=\"submit\">Connect</button></form></body></html>",
        s_prov_pin);
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* ================================================================
 *  HTTP 处理器 - 404 (强制门户兜底)
 * ================================================================ */

static esp_err_t http_404_handler(httpd_req_t *req, httpd_err_code_t err)
{
    (void)err;
    return http_get_root_handler(req);
}

/* ================================================================
 *  DNS 强制门户 (UDP :53)
 * ================================================================ */

static void dns_server_task(void *arg)
{
    (void)arg;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { vTaskDelete(NULL); return; }

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_port = htons(53),
        .sin_addr.s_addr = htonl(INADDR_ANY)
    };
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock);
        vTaskDelete(NULL);
        return;
    }
    s_dns_sock = sock;

    uint8_t buf[512], resp[512];
    while (s_provisioning) {
        struct sockaddr_in cli;
        socklen_t cli_len = sizeof(cli);
        int n = recvfrom(sock, buf, sizeof(buf), 0,
                         (struct sockaddr *)&cli, &cli_len);
        if (n < 12) continue;

        memcpy(resp, buf, 2);
        resp[2] = 0x81; resp[3] = 0x80;
        memcpy(resp + 4, buf + 4, 2);
        uint16_t qd = (buf[4] << 8) | buf[5];
        resp[6] = (qd >> 8); resp[7] = qd & 0xFF;
        resp[8] = 0; resp[9] = 0; resp[10] = 0; resp[11] = 0;

        int pos = 12;
        memcpy(resp + pos, buf + pos, n - pos);
        pos = n;

        resp[pos++] = 0xC0; resp[pos++] = 0x0C;
        resp[pos++] = 0x00; resp[pos++] = 0x01;
        resp[pos++] = 0x00; resp[pos++] = 0x01;
        resp[pos++] = 0x00; resp[pos++] = 0x00;
        resp[pos++] = 0x00; resp[pos++] = 0x3C;
        resp[pos++] = 0x00; resp[pos++] = 0x04;
        resp[pos++] = 192; resp[pos++] = 168;
        resp[pos++] = 4;   resp[pos++] = 1;

        sendto(sock, resp, pos, 0, (struct sockaddr *)&cli, cli_len);
    }

    close(sock);
    s_dns_sock = -1;
    vTaskDelete(NULL);
}

static void start_dns_server(void)
{
    xTaskCreate(dns_server_task, "dns_prov", 3584, NULL, 4, NULL);
}

/* ================================================================
 *  配网生命周期
 * ================================================================ */

static void provision_complete(bool success)
{
    s_provisioning = false;

    if (s_dns_sock >= 0) {
        close(s_dns_sock);
        s_dns_sock = -1;
    }

    if (s_httpd) {
        httpd_stop(s_httpd);
        s_httpd = NULL;
    }

    svc_ble_deinit();

    if (s_prov_cb) s_prov_cb(success);
    s_prov_cb = NULL;
}

/* ================================================================
 *  URI 路由表
 *  s_uri_connect.handler 由 setup_s_uri_connect_handler() 运行时赋值，
 *  注册本身由 register_connect_uri() 完成，无前向声明。
 * ================================================================ */

static httpd_uri_t s_uri_root = {
    .uri = "/", .method = HTTP_GET,
    .handler = http_get_root_handler
};
static httpd_uri_t s_uri_connect;

/* ================================================================
 *  HTTP 服务器
 * ================================================================ */

static void start_http_server(void)
{
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    cfg.lru_purge_enable = true;
    if (httpd_start(&s_httpd, &cfg) != ESP_OK) return;
    httpd_register_uri_handler(s_httpd, &s_uri_root);
    httpd_register_err_handler(s_httpd, HTTPD_404_NOT_FOUND,
                               http_404_handler);
}

/* ================================================================
 *  Soft-AP 管理
 * ================================================================ */

static void start_ap(void)
{
    static bool netif_created = false;

    if (!netif_created) {
        esp_netif_t *ap_netif = esp_netif_create_default_wifi_ap();
        esp_netif_ip_info_t ip = {
            .ip      = { .addr = esp_ip4addr_aton(AP_IP) },
            .gw      = { .addr = esp_ip4addr_aton(AP_IP) },
            .netmask = { .addr = esp_ip4addr_aton("255.255.255.0") }
        };
        esp_netif_dhcps_stop(ap_netif);
        esp_netif_set_ip_info(ap_netif, &ip);
        esp_netif_dhcps_start(ap_netif);
        netif_created = true;
    }

    esp_wifi_set_mode(WIFI_MODE_APSTA);

    wifi_config_t ap_cfg = {
        .ap = {
            .ssid = AP_SSID,
            .ssid_len = strlen(AP_SSID),
            .max_connection = 4,
            .authmode = WIFI_AUTH_OPEN
        }
    };
    esp_wifi_set_config(WIFI_IF_AP, &ap_cfg);
    esp_wifi_start();
}

/* ================================================================
 *  STA 连接尝试 (返回 bool，不自行重启 AP)
 * ================================================================ */

static bool try_sta_connect(const char *ssid, const char *pass)
{
    ESP_LOGI(TAG, "connecting to [%s]", ssid);
    svc_event_publish(EVT_WIFI_CONNECTING, (void *)ssid);
    esp_wifi_stop();
    esp_wifi_set_mode(WIFI_MODE_STA);

    wifi_config_t cfg = {0};
    strncpy((char *)cfg.sta.ssid, ssid, 32);
    strncpy((char *)cfg.sta.password, pass, 64);
    esp_wifi_set_config(WIFI_IF_STA, &cfg);
    esp_wifi_start();
    esp_wifi_connect();

    EventBits_t bits = xEventGroupWaitBits(s_evt,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE, pdFALSE, pdMS_TO_TICKS(15000));

    if (bits & WIFI_CONNECTED_BIT) {
        save_creds_to_nvs(ssid, pass);
        provision_complete(true);
        svc_event_publish(EVT_NETWORK_UP, NULL);
        return true;
    }
    return false;
}

/* ================================================================
 *  注册 /connect URI (仅注册，不设 handler)
 * ================================================================ */

static void register_connect_uri(void)
{
    httpd_register_uri_handler(s_httpd, &s_uri_connect);
}

/* ================================================================
 *  HTTP 处理器 - POST /connect (接收配网凭据)
 * ================================================================ */

static void url_decode(char *dst, const char *src, size_t dst_size)
{
    char hex[3] = {0};
    while (*src && dst_size > 1) {
        if (*src == '+') {
            *dst++ = ' ';
            src++;
            dst_size--;
        } else if (*src == '%'
                   && isxdigit((unsigned char)src[1])
                   && isxdigit((unsigned char)src[2])) {
            hex[0] = src[1]; hex[1] = src[2];
            *dst++ = (char)strtol(hex, NULL, 16);
            src += 3;
            dst_size--;
        } else {
            *dst++ = *src++;
            dst_size--;
        }
    }
    *dst = 0;
}

static esp_err_t http_post_connect_handler(httpd_req_t *req)
{
    char buf[256];
    int r = httpd_req_recv(req, buf, sizeof(buf) - 1);
    if (r <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    buf[r] = 0;

    char ssid[33] = {0}, pass[65] = {0}, pin[8] = {0};

    char *p, *e, tmp[256] = {0};
    size_t n;

    p = strstr(buf, "ssid=");
    if (p) {
        p += 5;
        e = strpbrk(p, "&\r\n");
        n = e ? (size_t)(e - p) : strlen(p);
        if (n < sizeof(tmp)) { memcpy(tmp, p, n); tmp[n] = 0; }
        url_decode(ssid, tmp, sizeof(ssid));
    }

    p = strstr(buf, "pass=");
    if (p) {
        p += 5;
        e = strpbrk(p, "&\r\n");
        n = e ? (size_t)(e - p) : strlen(p);
        if (n < sizeof(tmp)) { memcpy(tmp, p, n); tmp[n] = 0; }
        url_decode(pass, tmp, sizeof(pass));
    }

    p = strstr(buf, "pin=");
    if (p) {
        p += 4;
        e = strpbrk(p, "&\r\n");
        n = e ? (size_t)(e - p) : strlen(p);
        if (n < sizeof(tmp)) { memcpy(tmp, p, n); tmp[n] = 0; }
        url_decode(pin, tmp, sizeof(pin));
    }

    if (strlen(ssid) == 0 || strcmp(pin, s_prov_pin) != 0) {
        httpd_resp_send(req, "FAIL: bad ssid or pin", HTTPD_RESP_USE_STRLEN);
        return ESP_OK;
    }

    httpd_resp_send(req, "OK: connecting...", HTTPD_RESP_USE_STRLEN);

    if (try_sta_connect(ssid, pass)) return ESP_OK;

    ESP_LOGW(TAG, "connect failed, restart AP");
    if (s_httpd) { httpd_stop(s_httpd); s_httpd = NULL; }
    start_ap();
    start_http_server();
    register_connect_uri();
    svc_event_publish(EVT_WIFI_PROV_STARTED, NULL);
    return ESP_OK;
}

/* ================================================================
 *  设置 s_uri_connect.handler (依赖 http_post_connect_handler)
 * ================================================================ */

static void setup_s_uri_connect_handler(void)
{
    s_uri_connect.uri = "/connect";
    s_uri_connect.method = HTTP_POST;
    s_uri_connect.handler = http_post_connect_handler;
}

/* ================================================================
 *  BLE 凭据回调 (连接 + 失败回滚)
 * ================================================================ */

static void ble_cred_callback(const char *ssid, const char *pass)
{
    if (try_sta_connect(ssid, pass)) return;

    ESP_LOGW(TAG, "BLE connect failed, restart AP");
    if (s_httpd) { httpd_stop(s_httpd); s_httpd = NULL; }
    start_ap();
    start_http_server();
    register_connect_uri();
    svc_event_publish(EVT_WIFI_PROV_STARTED, NULL);
}

/* ================================================================
 *  WiFi 事件处理器
 * ================================================================ */

static void event_handler(void *arg, esp_event_base_t base,
                          int32_t id, void *data)
{
    (void)arg; (void)data;

    if (base == WIFI_EVENT) {
        if (id == WIFI_EVENT_STA_START) {
            if (!s_provisioning) esp_wifi_connect();
        } else if (id == WIFI_EVENT_STA_DISCONNECTED) {
            wifi_event_sta_disconnected_t *ev = (wifi_event_sta_disconnected_t *)data;
            ESP_LOGW(TAG, "STA disconnect reason=%d", ev ? ev->reason : -1);
            s_connected = false;
            if (!s_provisioning) {
                if (s_cb) s_cb(false);
                svc_event_publish(EVT_WIFI_DISCONNECTED, NULL);
                svc_event_publish(EVT_NETWORK_DOWN, NULL);
                wifi_config_t wcfg;
                esp_wifi_get_config(WIFI_IF_STA, &wcfg);
                ESP_LOGW(TAG, "WiFi disconnected, retry SSID=[%s] reason=%d", wcfg.sta.ssid, ev->reason);
                svc_event_publish(EVT_WIFI_CONNECTING, NULL);
                esp_wifi_connect();
            }
        }
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        s_connected = true;
        if (s_cb) s_cb(true);
        xEventGroupSetBits(s_evt, WIFI_CONNECTED_BIT);
        if (!s_provisioning) {
            svc_event_publish(EVT_WIFI_CONNECTED, NULL);
            svc_event_publish(EVT_NETWORK_UP, NULL);
            ESP_LOGI(TAG, "Got IP");
        }
    }
}

/* ================================================================
 *  公开 API
 * ================================================================ */

esp_err_t svc_wifi_init(void)
{
    s_evt = xEventGroupCreate();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                        event_handler, NULL, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                        event_handler, NULL, NULL);
    esp_wifi_set_mode(WIFI_MODE_STA);
    return ESP_OK;
}

esp_err_t svc_wifi_connect(const char *ssid, const char *pass,
                           svc_wifi_cb_t cb)
{
    s_cb = cb;
    wifi_config_t cfg = {0};
    strncpy((char *)cfg.sta.ssid, ssid, 32);
    strncpy((char *)cfg.sta.password, pass, 64);
    esp_wifi_set_config(WIFI_IF_STA, &cfg);
    esp_wifi_start();
    svc_event_publish(EVT_WIFI_CONNECTING, NULL);
    EventBits_t bits = xEventGroupWaitBits(s_evt,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE, pdFALSE, pdMS_TO_TICKS(15000));
    if (bits & WIFI_CONNECTED_BIT) {
        save_creds_to_nvs(ssid, pass);
        return ESP_OK;
    }
    return ESP_FAIL;
}

esp_err_t svc_wifi_disconnect(void)
{
    esp_wifi_stop();
    s_connected = false;
    return ESP_OK;
}

bool svc_wifi_is_connected(void)
{
    return s_connected;
}

esp_err_t svc_wifi_start_provisioning(svc_wifi_prov_cb_t cb)
{
    if (s_provisioning) return ESP_ERR_INVALID_STATE;

    s_provisioning = true;
    s_prov_cb = cb;

    generate_pin();
    start_ap();
    start_http_server();
    setup_s_uri_connect_handler();
    register_connect_uri();
    start_dns_server();
    svc_ble_init(ble_cred_callback, s_prov_pin);

    svc_event_publish(EVT_WIFI_PROV_STARTED, NULL);
    ESP_LOGI(TAG, "provisioning: PIN=%s AP=%s", s_prov_pin, AP_SSID);
    return ESP_OK;
}

esp_err_t svc_wifi_stop_provisioning(void)
{
    if (!s_provisioning) return ESP_ERR_INVALID_STATE;
    provision_complete(false);
    return ESP_OK;
}

bool svc_wifi_is_provisioning(void)
{
    return s_provisioning;
}

/* ================================================================
 *  服务生命周期
 * ================================================================ */

static esp_err_t svc_wifi_on_init(svc_base_t *svc, void *ctx)
{
    (void)svc; (void)ctx;
    return svc_wifi_init();
}

static esp_err_t svc_wifi_on_start(svc_base_t *svc)
{
    (void)svc;
    char ssid[33] = {0}, pass[65] = {0};

    if (load_creds_from_nvs(ssid, sizeof(ssid), pass, sizeof(pass))) {
        ESP_LOGI(TAG, "saved credentials: %s", ssid);
        return svc_wifi_connect(ssid, pass, NULL);
    }

    ESP_LOGI(TAG, "no saved credentials, start provisioning");
    return svc_wifi_start_provisioning(NULL);
}

static const char *s_wifi_deps[] = {"storage"};

svc_base_t g_svc_wifi = {
    .name = "wifi",
    .deps = s_wifi_deps,
    .dep_count = 1,
    .on_init = svc_wifi_on_init,
    .on_start = svc_wifi_on_start,
    .on_stop = NULL,
    .on_deinit = NULL,
};

const char *svc_wifi_get_prov_pin(void)
{
    return s_prov_pin;
}
