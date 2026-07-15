#include "framework/svc_function.h"
#include "os_task.h"
#include "os_event.h"
#include <stdlib.h>

typedef struct {
    svc_work_fn_t work;
    void *arg;
    esp_err_t result;
    os_event_handle_t event;
} svc_function_sync_ctx_t;

typedef struct {
    svc_work_fn_t work;
    void *arg;
} svc_function_async_ctx_t;

static void svc_function_sync_task(void *p)
{
    svc_function_sync_ctx_t *ctx = (svc_function_sync_ctx_t *)p;
    ctx->result = ctx->work(ctx->arg);
    os_event_set(ctx->event, 1);
    os_task_delete(NULL);
}

esp_err_t svc_function_call_sync(svc_work_fn_t work, void *arg, uint32_t timeout_ms)
{
    svc_function_sync_ctx_t ctx = { .work = work, .arg = arg, .result = ESP_FAIL };
    esp_err_t ret = os_event_create(&ctx.event);
    if (ret != ESP_OK) { return ret; }
    os_task_handle_t h;
    ret = os_task_create(svc_function_sync_task, "svcf_sync", 4096, &ctx, 5, &h, 0);
    if (ret != ESP_OK) { return ret; }
    os_event_wait(ctx.event, 1, true, false, timeout_ms);
    return ctx.result;
}

static void svc_function_async_task(void *p)
{
    svc_function_async_ctx_t *ctx = (svc_function_async_ctx_t *)p;
    ctx->work(ctx->arg);
    free(ctx);
    os_task_delete(NULL);
}

esp_err_t svc_function_call_async(svc_work_fn_t work, void *arg)
{
    svc_function_async_ctx_t *ctx = malloc(sizeof(*ctx));
    if (!ctx) { return ESP_ERR_NO_MEM; }
    ctx->work = work; ctx->arg = arg;
    os_task_handle_t h;
    esp_err_t ret = os_task_create(svc_function_async_task, "svcf_async", 4096, ctx, 5, &h, 0);
    if (ret != ESP_OK) { free(ctx); return ret; }
    return ESP_OK;
}
