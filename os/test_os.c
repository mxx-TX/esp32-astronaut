#include "test_os.h"
#include "os_task.h"
#include "os_queue.h"
#include "os_mutex.h"
#include "os_event.h"
#include "os_mem.h"
#include "esp_log.h"
#include <string.h>
static const char *TAG = "test_os";
#define T_CHECK(name, expr) do { esp_err_t r = (expr); ESP_LOGI(TAG, "[%s] %s", (r == ESP_OK) ? "PASS" : "FAIL", name); } while(0)
static int s_task_ran = 0;
static void test_task_func(void *arg) { s_task_ran = 1; os_task_delete(NULL); }
esp_err_t test_os_run_all(void)
{
    ESP_LOGI(TAG, "========== OS Test Suite ==========");
    os_task_handle_t th;
    T_CHECK("task create", os_task_create(test_task_func, "tt", 2048, NULL, 2, &th, 0));
    os_task_delay_ms(200);
    T_CHECK("task ran", s_task_ran ? ESP_OK : ESP_FAIL);

    os_queue_handle_t qh;
    T_CHECK("queue create", os_queue_create(sizeof(int), 4, &qh));
    int val = 42, out = 0;
    T_CHECK("queue send", os_queue_send(qh, &val, 100));
    T_CHECK("queue recv", os_queue_recv(qh, &out, 100));
    T_CHECK("queue val", (out == 42) ? ESP_OK : ESP_FAIL);

    os_mutex_handle_t mh;
    T_CHECK("mutex create", os_mutex_create(&mh));
    T_CHECK("mutex lock", os_mutex_lock(mh, 100));
    T_CHECK("mutex unlock", os_mutex_unlock(mh));

    os_event_handle_t eh;
    T_CHECK("event create", os_event_create(&eh));
    T_CHECK("event set", os_event_set(eh, OS_EVENT_BIT(0)));
    T_CHECK("event wait", os_event_wait(eh, OS_EVENT_BIT(0), true, true, 100));

    void *p = os_mem_malloc(128);
    T_CHECK("mem malloc", p ? ESP_OK : ESP_FAIL);
    os_mem_free(p);
    void *p2 = os_mem_calloc(4, 32);
    T_CHECK("mem calloc", p2 ? ESP_OK : ESP_FAIL);
    os_mem_free(p2);

    ESP_LOGI(TAG, "========== OS Test Done ==========");
    return ESP_OK;
}
