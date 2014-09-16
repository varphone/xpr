#ifndef XPR_SYS_H
#define XPR_SYS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void XPR_SYS_Poweroff(void);
void XPR_SYS_Reboot(void);
int XPR_SYS_EnableThreadRealtimeSchedule(int64_t tid);
int XPR_SYS_EnableProcessRealtimeSchedule(int64_t pid);
int XPR_SYS_SetAudioClockFrequency(int freq);

#ifdef __cplusplus
}
#endif

#endif // XPR_SYS_H

