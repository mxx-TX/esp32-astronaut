#ifndef APP_FSM_H
#define APP_FSM_H
#include "app_events.h"
#include "esp_err.h"
typedef enum {
    APP_STATE_BOOT,
    APP_STATE_IDLE,
    APP_STATE_LISTENING,
    APP_STATE_THINKING,
    APP_STATE_SPEAKING,
} app_state_t;
esp_err_t app_fsm_init(void);
app_state_t app_fsm_get_state(void);
void app_fsm_task(void *arg);
esp_err_t app_fsm_post_event(app_event_t evt);
#endif
