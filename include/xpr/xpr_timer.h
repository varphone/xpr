/*
 * File: xpr_timer.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  Timer and Timer Queue
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2019-07-02 22:22:48 +08:00 Tuesday, 02 July
 * Last Modified : 2019-07-02 22:30:40 +08:00 Tuesday, 02 July
 * Modified By   : Varphone Wong <varphone@qq.com>
 * ---------------------------------------------------------------------------
 * Copyright (C) 2010 - 2019 Varphone Wong, Varphone.com
 * All rights reserved.
 * ---------------------------------------------------------------------------
 * HISTORY:
 */
#ifndef XPR_TIMER_H
#define XPR_TIMER_H

#include <stdint.h>
#include <xpr/xpr_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_TIMER_TYPE_DEFINED
#define XPR_TIMER_TYPE_DEFINED
struct XPR_Timer;
typedef struct XPR_Timer XPR_Timer;
#endif // XPR_TIMER_TYPE_DEFINED

#ifndef XPR_TIMER_ID_TYPE_DEFINED
#define XPR_TIMER_ID_TYPE_DEFINED
typedef int XPR_TimerId;
#endif // XPR_TIMER_TYPE_DEFINED

#ifndef XPR_TIMERQUEUE_TYPE_DEFINED
#define XPR_TIMERQUEUE_TYPE_DEFINED
struct XPR_TimerQueue;
typedef struct XPR_TimerQueue XPR_TimerQueue;
#endif // XPR_TIMER_TYPE_DEFINED

#ifndef XPR_TIMER_RETURN_TYPE_DEFINED
#define XPR_TIMER_RETURN_TYPE_DEFINED
typedef enum XPR_TimerReturn {
    XPR_TIMER_CONTINUE,
    XPR_TIMER_SUSPEND,
    XPR_TIMER_TERMINATE,
} XPR_TimerReturn;
#endif // XPR_TIMER_RETURN_TYPE_DEFINED

/// Function to callback if timer expired.
/// \param [out] opaque     The user context data.
/// \return sa #XPR_TimerReturn
typedef XPR_TimerReturn (*XPR_TimerFn)(void* opaque);

XPR_API XPR_Timer* XPR_TimerCreate(XPR_TimerId id, int64_t interval,
                                   XPR_TimerFn, void* opaque);

XPR_API void XPR_TimerDestroy(XPR_Timer* self);

XPR_API void XPR_TimerEnable(XPR_Timer* self);

XPR_API void XPR_TimerDisable(XPR_Timer* self);

XPR_API void XPR_TimerSetInterval(XPR_Timer* self, int64_t interval);

XPR_API XPR_TimerQueue* XPR_TimerQueueCreate(void);

XPR_API void XPR_TimerQueueDestroy(XPR_TimerQueue* self);

XPR_API int XPR_TimerQueueAdd(XPR_TimerQueue* self, XPR_Timer* timer);

XPR_API XPR_Timer* XPR_TimerQueueFind(XPR_TimerQueue* self, XPR_TimerId id);

XPR_API int XPR_TimerQueueRemove(XPR_TimerQueue* self, XPR_Timer* timer);

#ifdef __cplusplus
}
#endif

#endif // XPR_TIMER_H
