#ifndef XPR_SYS_H
#define XPR_SYS_H

#include <stdint.h>
#include <xpr/xpr_common.h>

#ifdef __cplusplus
extern "C" {
#endif

XPR_API void XPR_SYS_Poweroff(void);
XPR_API void XPR_SYS_Reboot(void);
XPR_API void XPR_SYS_DelayedReboot(int secs);
XPR_API int XPR_SYS_EnableThreadRealtimeSchedule(int64_t tid);
XPR_API int XPR_SYS_EnableProcessRealtimeSchedule(int64_t pid);
XPR_API int XPR_SYS_SetAudioClockFrequency(int freq);
XPR_API int64_t XPR_SYS_GetCTS(void);

#ifdef __cplusplus
}
#endif

#endif // XPR_SYS_H

