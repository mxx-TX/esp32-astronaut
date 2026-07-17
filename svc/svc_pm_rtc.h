#ifndef SVC_PM_RTC_H
#define SVC_PM_RTC_H
#include <stdint.h>

#define PM_RTC_MAGIC  0x504D444C  /* "PMDL" */

typedef struct {
    uint32_t    magic;          /* PM_RTC_MAGIC */
    uint32_t    boot_mode;      /* 0=cold boot 1=deep sleep wake */
    uint32_t    wakeup_count;   /* deep sleep wakeup count */
    uint32_t    crc32;          /* checksum */
} pm_rtc_data_t;

extern pm_rtc_data_t g_pm_rtc;
#endif