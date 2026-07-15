#include "test_svc.h"
#include "framework/svc_manager.h"
#include "framework/svc_event.h"
#include "framework/svc_function.h"
#include "svc_storage.h"
#include "svc_indicator.h"
#include "svc_audio.h"
#include "bsp_audio.h"
 #include "os_task.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <string.h>
#include <math.h>
#include <stdlib.h>

static const char *TAG = "test_svc";
#define T_CHECK(name, expr) do { esp_err_t r = (expr); ESP_LOGI(TAG, "[%s] %s", (r == ESP_OK) ? "PASS" : "FAIL", name); } while(0)

static bool s_evt_test_ok = false;

static void test_event_handler(svc_event_id_t id, void *data, void *ctx)
{
    (void)data; (void)ctx;
    if (id == EVT_USER) { s_evt_test_ok = true; }
}

static esp_err_t test_async_work(void *arg)
{
    *(int *)arg = 42;
    return ESP_OK;
}

static void test_audio_beep(bsp_handles_t *h)
{
#define BEEP_FREQ   440
#define BEEP_RATE   48000
#define BEEP_MS     600
#define BEEP_SAMPS  (BEEP_RATE * BEEP_MS / 1000)

    ESP_LOGI(TAG, "--- Audio Beep Test (440Hz, %dms) ---", BEEP_MS);

    int frame_bytes = BEEP_SAMPS * 4;
    int16_t *buf = malloc(frame_bytes);
    if (!buf) { ESP_LOGE(TAG, "alloc fail"); return; }

    double amp = 4000.0;
    for (int i = 0; i < BEEP_SAMPS; i++) {
        int16_t s = (int16_t)(amp * sin(2.0 * 3.14159265 * BEEP_FREQ * i / BEEP_RATE));
        buf[i * 2]     = s;
        buf[i * 2 + 1] = s;
    }

    svc_audio_set_vol(h->i2c_dac, 60);
    vTaskDelay(pdMS_TO_TICKS(50));

    esp_err_t ret = svc_audio_play(h->i2s_tx, (uint8_t *)buf, frame_bytes, 2000);
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "[PASS] audio beep played");
    } else {
        ESP_LOGE(TAG, "[FAIL] audio beep: 0x%x", ret);
    }

    free(buf);
}

/* 找 16bit PCM 数据的最大最小值 */
static void codec_max_sample(const uint8_t *data, int size, int *max_val, int *min_val)
{
    const int16_t *s = (const int16_t *)data;
    int n = size >> 1;
    int max = s[0], min = s[0];
    for (int i = 1; i < n; i++) {
        if (s[i] > max) max = s[i];
        else if (s[i] < min) min = s[i];
    }
    *max_val = max;
    *min_val = min;
}

/* 实时回声环回：MIC -> Speaker，10 秒 */
static void test_audio_loopback(bsp_handles_t *h)
{
#define REC_MS          500
#define CHUNK_SIZE      512

    ESP_LOGI(TAG, "--- Audio Record & Playback (500ms loop) ---");

    int total_bytes = REC_MS * 48000 * 4 / 1000;
    uint8_t *rec = malloc(total_bytes);
    if (!rec) { ESP_LOGE(TAG, "alloc fail"); return; }
    uint8_t *chunk = malloc(CHUNK_SIZE);
    if (!chunk) { free(rec); ESP_LOGE(TAG, "alloc fail"); return; }

    svc_audio_set_vol(h->i2c_dac, 60);
    os_task_delay_ms(50);
    for (int loop = 0; loop < 3; loop++) {
    ESP_LOGI(TAG, "Loop %d: recording...", loop + 1);
    int recd = 0;
    while (recd < total_bytes) {
        size_t read = 0;
        esp_err_t ret = svc_audio_record(h->i2s_rx, chunk, CHUNK_SIZE, &read, 1000);
        if (ret != ESP_OK) { ESP_LOGE(TAG, "record fail: 0x%x", ret); break; }
        if (read == 0) { continue; }
        if (recd + (int)read <= total_bytes) {
            memcpy(rec + recd, chunk, read);
        }
        recd += (int)read;
    }

    int max_s, min_s;
    codec_max_sample(rec, recd, &max_s, &min_s);
    ESP_LOGI(TAG, "  recorded %d bytes, range [%d, %d]", recd, min_s, max_s);

    ESP_LOGI(TAG, "  playing...");
    int played = 0;
    while (played < recd) {
        int left = recd - played;
        int now = (left > CHUNK_SIZE) ? CHUNK_SIZE : left;
        esp_err_t ret = svc_audio_play(h->i2s_tx, rec + played, now, 2000);
        if (ret != ESP_OK) { ESP_LOGE(TAG, "play fail: 0x%x", ret); break; }
        played += now;
    }

    }

    free(chunk);
    free(rec);
    ESP_LOGI(TAG, "--- Record & Playback done (3 loops) ---");
}

esp_err_t test_svc_run_all(bsp_handles_t *h)
{
    ESP_LOGI(TAG, "========== SVC Test Suite ==========");

    svc_base_t *s = svc_manager_find("storage");
    T_CHECK("storage inited", (s && s->state == SVC_STARTED) ? ESP_OK : ESP_FAIL);

    s = svc_manager_find("wifi");
    T_CHECK("wifi inited", (s && s->state == SVC_STARTED) ? ESP_OK : ESP_FAIL);

    T_CHECK("NVS set str", svc_storage_nvs_set_str("test_key", "hello"));
    char buf[32]; size_t len = sizeof(buf);
    T_CHECK("NVS get str", svc_storage_nvs_get_str("test_key", buf, &len));
    ESP_LOGI(TAG, "NVS read: %s", buf);

    T_CHECK("Indicator slow", svc_indicator_set(IND_SLOW));
    vTaskDelay(pdMS_TO_TICKS(500));
    T_CHECK("Indicator fast", svc_indicator_set(IND_FAST));
    vTaskDelay(pdMS_TO_TICKS(500));
    T_CHECK("Indicator off", svc_indicator_set(IND_OFF));

    test_audio_beep(h);
    test_audio_loopback(h);

    ESP_LOGI(TAG, "--- Framework Tests ---");
    svc_event_subscribe(EVT_USER, test_event_handler, NULL);
    svc_event_publish(EVT_USER, NULL);
    T_CHECK("event pub/sub", s_evt_test_ok ? ESP_OK : ESP_FAIL);
    svc_event_unsubscribe(EVT_USER, test_event_handler);

    int async_val = 0;
    T_CHECK("function call sync", svc_function_call_sync(test_async_work, &async_val, 5000));
    T_CHECK("function sync result", (async_val == 42) ? ESP_OK : ESP_FAIL);
    T_CHECK("function call async", svc_function_call_async(test_async_work, &async_val));

    ESP_LOGI(TAG, "========== SVC Test Done ==========");
    return ESP_OK;
}
