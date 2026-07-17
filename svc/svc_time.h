#ifndef SVC_TIME_H
#define SVC_TIME_H
#include "framework/svc_manager.h"
#include <stdbool.h>
#include <stddef.h>

void svc_time_format_time(char *buf, size_t len);
void svc_time_format_date(char *buf, size_t len);
void svc_time_format_weekday(char *buf, size_t len);
bool svc_time_is_synced(void);

extern svc_base_t g_svc_time;

#endif