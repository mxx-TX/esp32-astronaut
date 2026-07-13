#include "svc_http.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include <string.h>
esp_err_t svc_http_post(const char *url, const char *api_key, const uint8_t *body, size_t body_len, uint8_t *resp, size_t *resp_len, size_t max_resp)
{
    esp_http_client_config_t cfg = {
        .url = url,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 15000,
    };
    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    if (!client) return ESP_FAIL;
    esp_http_client_set_header(client, "Content-Type", "application/json");
    if (api_key) {
        char hdr[256];
        snprintf(hdr, sizeof(hdr), "Bearer %s", api_key);
        esp_http_client_set_header(client, "Authorization", hdr);
    }
    esp_err_t ret = esp_http_client_open(client, body_len);
    if (ret != ESP_OK) { esp_http_client_cleanup(client); return ret; }
    int wrote = esp_http_client_write(client, (const char *)body, body_len);
    if (wrote < 0) { esp_http_client_cleanup(client); return ESP_FAIL; }
    int content_len = esp_http_client_fetch_headers(client);
    *resp_len = 0;
    if (content_len > 0) {
        int r = esp_http_client_read_response(client, (char *)resp, max_resp);
        if (r > 0) *resp_len = (size_t)r;
    }
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return ESP_OK;
}
esp_err_t svc_http_download(const char *url, uint8_t *out, size_t *out_len, size_t max_len)
{
    esp_http_client_config_t cfg = { .url = url, .method = HTTP_METHOD_GET, .timeout_ms = 15000 };
    esp_http_client_handle_t client = esp_http_client_init(&cfg);
    if (!client) return ESP_FAIL;
    esp_err_t ret = esp_http_client_open(client, 0);
    if (ret != ESP_OK) { esp_http_client_cleanup(client); return ret; }
    int content_len = esp_http_client_fetch_headers(client);
    *out_len = 0;
    if (content_len > 0) {
        int r = esp_http_client_read_response(client, (char *)out, max_len);
        if (r > 0) *out_len = (size_t)r;
    }
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return ESP_OK;
}
