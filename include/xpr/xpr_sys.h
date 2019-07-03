/*
 * File: xpr_sys.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 系统相关操作接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2014-11-21 12:50:43 Friday, 21 November
 * Last Modified : 2019-07-03 04:36:08 Wednesday, 3 July
 * Modified By   : Varphone Wong <varphone@qq.com>
 * ---------------------------------------------------------------------------
 * Copyright (C) 2012 - 2019 CETC55, Technology Development CO.,LTD.
 * Copyright (C) 2012 - 2019 Varphone Wong, Varphone.com.
 * All rights reserved.
 * ---------------------------------------------------------------------------
 * HISTORY:
 * 2019-07-03   varphone    更新版权信息
 * 2014-11-21   varphone    初始版本建立
 */
#ifndef XPR_SYS_H
#define XPR_SYS_H

#include <stdint.h>
#include <xpr/xpr_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XPR_SYS_CTS_UNIT 1000000

XPR_API void XPR_SYS_Poweroff(void);
XPR_API void XPR_SYS_Reboot(void);
XPR_API void XPR_SYS_DelayedReboot(int secs);
XPR_API int XPR_SYS_EnableThreadRealtimeSchedule(int64_t tid);
XPR_API int XPR_SYS_EnableProcessRealtimeSchedule(int64_t pid);
XPR_API int XPR_SYS_SetAudioClockFrequency(int freq);
XPR_API int64_t XPR_SYS_GetCTS(void);
XPR_API int XPR_SYS_WaitKey(int timeout);

#ifdef __cplusplus
}
#endif

#endif // XPR_SYS_H
