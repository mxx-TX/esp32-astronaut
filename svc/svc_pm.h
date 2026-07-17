#ifndef SVC_PM_H
#define SVC_PM_H
#include "framework/svc_manager.h"

void svc_pm_activity(void);
void svc_pm_request_deep_sleep(void);
extern svc_base_t g_svc_pm;

#endif