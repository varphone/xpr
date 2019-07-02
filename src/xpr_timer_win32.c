/*
 * File: xpr_timer_win32.c
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
#include <stdlib.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_timer.h>

XPR_API XPR_Timer* XPR_TimerCreate(XPR_TimerId id, int64_t interval,
                                   XPR_TimerFn, void* opaque)
{
    // FIXME:
    return NULL;
}

XPR_API void XPR_TimerDestroy(XPR_Timer* self)
{
    // FIXME:
}

XPR_API void XPR_TimerEnable(XPR_Timer* self)
{
    // FIXME:
}

XPR_API void XPR_TimerDisable(XPR_Timer* self)
{
    // FIXME:
}

XPR_API void XPR_TimerSetInterval(XPR_Timer* self, int64_t interval)
{
    // FIXME:
}

XPR_API XPR_TimerQueue* XPR_TimerQueueCreate(void)
{
    // FIXME:
    return NULL;
}

XPR_API void XPR_TimerQueueDestroy(XPR_TimerQueue* self)
{
    // FIXME:
}

XPR_API int XPR_TimerQueueAdd(XPR_TimerQueue* self, XPR_Timer* timer)
{
    // FIXME:
    return XPR_ERR_ERROR;
}

XPR_API XPR_Timer* XPR_TimerQueueFind(XPR_TimerQueue* self, XPR_TimerId id)
{
    // FIXME:
    return NULL;
}

XPR_API int XPR_TimerQueueRemove(XPR_TimerQueue* self, XPR_Timer* timer)
{
    // FIXME:
    return XPR_ERR_ERROR;
}
