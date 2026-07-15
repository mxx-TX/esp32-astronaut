#ifndef SVC_MANAGER_H
#define SVC_MANAGER_H
#include "esp_err.h"
#include <stdint.h>

typedef enum { SVC_UNINIT, SVC_INITED, SVC_STARTED } svc_state_t;

typedef struct svc_base {
    const char *name;
    const char **deps;
    int dep_count;
    svc_state_t state;
    esp_err_t (*on_init)(struct svc_base *svc, void *ctx);
    esp_err_t (*on_start)(struct svc_base *svc);
    esp_err_t (*on_stop)(struct svc_base *svc);
    esp_err_t (*on_deinit)(struct svc_base *svc);
} svc_base_t;

esp_err_t svc_manager_register(svc_base_t *svc);
esp_err_t svc_manager_init_all(void *ctx);
esp_err_t svc_manager_start_all(void);
esp_err_t svc_manager_stop_all(void);
esp_err_t svc_manager_deinit_all(void);
svc_base_t *svc_manager_find(const char *name);

#endif
