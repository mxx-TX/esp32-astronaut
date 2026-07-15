#include "framework/svc_manager.h"
#include "esp_log.h"
#include <string.h>

#define MAX_SVCS 16
static svc_base_t *s_svcs[MAX_SVCS];
static int s_count = 0;
static const char *TAG = "svc_mgr";

esp_err_t svc_manager_register(svc_base_t *svc)
{
    if (s_count >= MAX_SVCS) { return ESP_ERR_NO_MEM; }
    s_svcs[s_count++] = svc;
    svc->state = SVC_UNINIT;
    return ESP_OK;
}

esp_err_t svc_manager_init_all(void *ctx)
{
    int in_deg[MAX_SVCS] = {0};
    bool done[MAX_SVCS] = {false};
    int done_cnt = 0;
    for (int i = 0; i < s_count; i++) {
        in_deg[i] = s_svcs[i]->dep_count;
    }
    while (done_cnt < s_count) {
        bool found = false;
        for (int i = 0; i < s_count; i++) {
            if (!done[i] && in_deg[i] == 0) {
                svc_base_t *svc = s_svcs[i];
                if (svc->on_init) {
                    esp_err_t ret = svc->on_init(svc, ctx);
                    if (ret != ESP_OK) { ESP_LOGE(TAG, "init %s fail: 0x%x", svc->name, ret); return ret; }
                }
                svc->state = SVC_INITED;
                done[i] = true; done_cnt++; found = true;
                for (int j = 0; j < s_count; j++) {
                    if (!done[j]) {
                        for (int k = 0; k < s_svcs[j]->dep_count; k++) {
                            if (strcmp(s_svcs[j]->deps[k], svc->name) == 0) { in_deg[j]--; }
                        }
                    }
                }
                break;
            }
        }
        if (!found) { ESP_LOGE(TAG, "cycle in deps"); return ESP_ERR_INVALID_STATE; }
    }
    ESP_LOGI(TAG, "all %d inited", s_count);
    return ESP_OK;
}

esp_err_t svc_manager_start_all(void)
{
    for (int i = 0; i < s_count; i++) {
        if (s_svcs[i]->state != SVC_INITED) { continue; }
        if (s_svcs[i]->on_start) {
            esp_err_t ret = s_svcs[i]->on_start(s_svcs[i]);
            if (ret != ESP_OK) { return ret; }
        }
        s_svcs[i]->state = SVC_STARTED;
    }
    ESP_LOGI(TAG, "all %d started", s_count);
    return ESP_OK;
}

esp_err_t svc_manager_stop_all(void)
{
    for (int i = s_count - 1; i >= 0; i--) {
        if (s_svcs[i]->state != SVC_STARTED) { continue; }
        if (s_svcs[i]->on_stop) { s_svcs[i]->on_stop(s_svcs[i]); }
        s_svcs[i]->state = SVC_INITED;
    }
    return ESP_OK;
}

esp_err_t svc_manager_deinit_all(void)
{
    for (int i = s_count - 1; i >= 0; i--) {
        if (s_svcs[i]->on_deinit) { s_svcs[i]->on_deinit(s_svcs[i]); }
        s_svcs[i]->state = SVC_UNINIT;
    }
    return ESP_OK;
}

svc_base_t *svc_manager_find(const char *name)
{
    for (int i = 0; i < s_count; i++) {
        if (strcmp(s_svcs[i]->name, name) == 0) { return s_svcs[i]; }
    }
    return NULL;
}
