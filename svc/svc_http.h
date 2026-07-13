#ifndef SVC_HTTP_H
#define SVC_HTTP_H
#include "esp_err.h"
#include <stddef.h>
esp_err_t svc_http_post(const char *url, const char *api_key, const uint8_t *body, size_t body_len, uint8_t *resp, size_t *resp_len, size_t max_resp);
esp_err_t svc_http_download(const char *url, uint8_t *out, size_t *out_len, size_t max_len);
#endif
