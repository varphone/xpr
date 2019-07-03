/*
 * File: xpr_timer.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  Timer and Timer Queue
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2019-07-02 22:22:48 +08:00 Tuesday, 02 July
 * Last Modified : 2019-07-03 22:30:40 +08:00 Wednesday, 03 July
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
#define XPR_TIMER_ID_SINGLESHOT 0xFFFFFFFF
#endif // XPR_TIMER_TYPE_DEFINED

#ifndef XPR_TIMERQUEUE_TYPE_DEFINED
#define XPR_TIMERQUEUE_TYPE_DEFINED
struct XPR_TimerQueue;
typedef struct XPR_TimerQueue XPR_TimerQueue;
#endif // XPR_TIMER_TYPE_DEFINED

#ifndef XPR_TIMER_RETURN_TYPE_DEFINED
#define XPR_TIMER_RETURN_TYPE_DEFINED
typedef enum XPR_TimerReturn
{
    XPR_TIMER_CONTINUE,  ///< Reset the timer for next time
    XPR_TIMER_SUSPEND,   ///< Put timer in suspend state
    XPR_TIMER_TERMINATE, ///< Terminate the timer now
} XPR_TimerReturn;
#endif // XPR_TIMER_RETURN_TYPE_DEFINED

/// Function to callback if timer expired.
/// \param [out] opaque     The user context data.
/// \return sa #XPR_TimerReturn
typedef XPR_TimerReturn (*XPR_TimerFn)(void* opaque);

/// Create a new timer with disabled initialize state.
/// \param [in] id          To identify the timer, must be unique to each timers.
/// \param [in] interval    Trigger interval in us.
/// \param [in] fn          Function to callack if triggered.
/// \param [in] opaque      User context data for callback.
/// \note The XPR_TimerId == 0xFFFFFFFF is used for Single Shot Timers.
XPR_API XPR_Timer* XPR_TimerCreate(XPR_TimerId id, int64_t interval,
                                   XPR_TimerFn fn, void* opaque);

/// Destroy the timer and remove from the timer queue if added.
XPR_API void XPR_TimerDestroy(XPR_Timer* self);

/// Enable the timer and resume from the timer queue if added.
XPR_API void XPR_TimerEnable(XPR_Timer* self);

/// Disable the timer and suspend from the timer queue if added.
XPR_API void XPR_TimerDisable(XPR_Timer* self);

/// Return the id of the timer.
XPR_API XPR_TimerId XPR_TimerGetId(XPR_Timer* self);

/// Return the interval of the timer.
XPR_API int64_t XPR_TimerGetInterval(XPR_Timer* self);

// Return XPR_TRUE if the timer enabled, otherwise return XPR_FALSE.
XPR_API int XPR_TimerIsEnabled(XPR_Timer* self);

/// Set the timer inteval in us.
XPR_API void XPR_TimerSetInterval(XPR_Timer* self, int64_t interval);

/// Signal the TimerQueue destroy the Timer on last expired.
XPR_API void XPR_TimerTerminate(XPR_Timer* self);

/// Create a new timer queue.
XPR_API XPR_TimerQueue* XPR_TimerQueueCreate(void);

/// Destroy the time queue.
XPR_API void XPR_TimerQueueDestroy(XPR_TimerQueue* self);

/// Add timer to the timer queue.
/// \retval XPR_ERR_OK      Success
/// \retval Others          Error
XPR_API int XPR_TimerQueueAdd(XPR_TimerQueue* self, XPR_Timer* timer);

/// Create a timer and add to the timer queue.
/// \retval XPR_ERR_OK      Success
/// \retval Others          Error
XPR_API int XPR_TimerQueueAddNew(XPR_TimerQueue* self, XPR_TimerId id,
                                 int64_t interval, XPR_TimerFn fn,
                                 void* opaque);

/// Return the id matched timer from the timer queue.
XPR_API XPR_Timer* XPR_TimerQueueFindId(XPR_TimerQueue* self, XPR_TimerId id);

/// Remove and destroy the timer from the timer queue.
/// \retval XPR_ERR_OK      Success
/// \retval Others          Error
XPR_API int XPR_TimerQueueRemove(XPR_TimerQueue* self, XPR_Timer* timer);

/// Remove and destroy the id matched timer from the timer queue.
/// \retval XPR_ERR_OK      Success
/// \retval Others          Error
XPR_API int XPR_TimerQueueRemoveId(XPR_TimerQueue* self, XPR_TimerId id);

/// Resume the timer from the timer queue.
/// \retval XPR_ERR_OK      Success
/// \retval Others          Error
XPR_API int XPR_TimerQueueResume(XPR_TimerQueue* self, XPR_Timer* timer);

/// Resume the id matched timer from the timer queue.
/// \retval XPR_ERR_OK      Success
/// \retval Others          Error
XPR_API int XPR_TimerQueueResumeId(XPR_TimerQueue* self, XPR_TimerId id);

/// Schedule a single shot timer to the timer queue.
/// \note The timer will be remove and destroy after expired.
/// \sa #XPR_TimerCreate
/// \retval XPR_ERR_OK      Success
/// \retval Others          Error
XPR_API int XPR_TimerQueueSingleShot(XPR_TimerQueue* self, int64_t interval,
                                     XPR_TimerFn fn, void* opaque);

/// Suspend the timer from timer queue.
/// \retval XPR_ERR_OK      Success
/// \retval Others          Error
XPR_API int XPR_TimerQueueSuspend(XPR_TimerQueue* self, XPR_Timer* timer);

/// Suspend the id matched timer from timer queue.
/// \retval XPR_ERR_OK      Success
/// \retval Others          Error
XPR_API int XPR_TimerQueueSuspendId(XPR_TimerQueue* self, XPR_TimerId id);

/// Alias to #XPR_TimerQueueRemoveId
#define XPR_TimerQueueTerminateId XPR_TimerQueueRemoveId

#ifdef __cplusplus
}
#endif

#endif // XPR_TIMER_H
