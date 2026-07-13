#include "app_fsm.h"
#include "os_queue.h"
#include "esp_log.h"
static const char *TAG = "app_fsm";
static app_state_t s_state = APP_STATE_BOOT;
static os_queue_handle_t s_evt_queue;
esp_err_t app_fsm_init(void)
{
    s_state = APP_STATE_BOOT;
    esp_err_t ret = os_queue_create(sizeof(app_event_t), 16, &s_evt_queue);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "queue fail"); return ret; }
    ESP_LOGI(TAG, "FSM init, state=BOOT");
    return ESP_OK;
}
app_state_t app_fsm_get_state(void) { return s_state; }
esp_err_t app_fsm_post_event(app_event_t evt)
{
    if (!s_evt_queue) return ESP_FAIL;
    return os_queue_send(s_evt_queue, &evt, 100);
}
static void fsm_transition(app_event_t evt)
{
    switch (s_state) {
    case APP_STATE_BOOT:
        if (evt == APP_EVT_BOOT_DONE) s_state = APP_STATE_IDLE;
        break;
    case APP_STATE_IDLE:
        if (evt == APP_EVT_WAKE_WORD || evt == APP_EVT_TOUCH) s_state = APP_STATE_LISTENING;
        break;
    case APP_STATE_LISTENING:
        if (evt == APP_EVT_ASR_DONE) s_state = APP_STATE_THINKING;
        else if (evt == APP_EVT_TIMEOUT) s_state = APP_STATE_IDLE;
        break;
    case APP_STATE_THINKING:
        if (evt == APP_EVT_LLM_DONE) s_state = APP_STATE_SPEAKING;
        else if (evt == APP_EVT_TIMEOUT) s_state = APP_STATE_IDLE;
        break;
    case APP_STATE_SPEAKING:
        if (evt == APP_EVT_TTS_DONE || evt == APP_EVT_TIMEOUT) s_state = APP_STATE_IDLE;
        break;
    }
}
void app_fsm_task(void *arg)
{
    while (1) {
        app_event_t evt;
        if (os_queue_recv(s_evt_queue, &evt, portMAX_DELAY) == ESP_OK) {
            ESP_LOGI(TAG, "evt=%d state=%d", evt, s_state);
            fsm_transition(evt);
            ESP_LOGI(TAG, "-> state=%d", s_state);
        }
    }
}
