#ifndef SVC_FUNCTION_H
#define SVC_FUNCTION_H
#include "esp_err.h"
#include <stdint.h>

typedef esp_err_t (*svc_work_fn_t)(void *arg);

esp_err_t svc_function_call_sync(svc_work_fn_t work, void *arg, uint32_t timeout_ms);
esp_err_t svc_function_call_async(svc_work_fn_t work, void *arg);

#endif
