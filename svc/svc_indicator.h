#ifndef SVC_INDICATOR_H
#define SVC_INDICATOR_H
#include "framework/svc_manager.h"
#include "esp_err.h"
typedef enum { IND_OFF, IND_ON, IND_SLOW, IND_FAST, IND_BREATHE } svc_ind_mode_t;
esp_err_t svc_indicator_set(svc_ind_mode_t mode);

extern svc_base_t g_svc_indicator;

#endif
