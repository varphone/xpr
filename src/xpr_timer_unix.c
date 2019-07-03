/*
 * File: xpr_timer_unix.c
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
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_list.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_sync.h>
#include <xpr/xpr_sys.h>
#include <xpr/xpr_timer.h>
#include <xpr/xpr_thread.h>

#define XPR_TIMER_FLAG_ENABLED 0x0001
#define XPR_TIMER_FLAG_TERMINATE 0x0002

struct XPR_Timer
{
    XPR_ListNodeSL _base; // SinglyLinked list node base
    XPR_TimerId id;       // Timer identifer
    int64_t interval;     // Interval in us
    XPR_TimerFn fn;       // Callback on expired
    void* opaque;         // User context data for callback
    int64_t expire;       // Expire timestamp
    int flags;            // Flags to control the timer
};

struct XPR_TimerQueue
{
    int exitLoop;
    XPR_List* activeTimers;
    XPR_List* suspendTimers;
    pthread_mutex_t lock;
    pthread_cond_t schedule;
    XPR_Thread* thread;
};

static void* nodeAlloc(void)
{
    XPR_Timer* timer = XPR_Alloc(sizeof(XPR_Timer));
    memset(timer, 0, sizeof(*timer));
    return timer;
}

static void nodeFree(void* node)
{
    XPR_Free(node);
}

static int nodeCompare(void* a, void* b)
{
    XPR_Timer* ta = (XPR_Timer*)(a);
    XPR_Timer* tb = (XPR_Timer*)(b);
    if (ta->expire < tb->expire)
        return -1;
    if (ta->expire > tb->expire)
        return 0;
    return 0;
}

XPR_API XPR_Timer* XPR_TimerCreate(XPR_TimerId id, int64_t interval, XPR_TimerFn fn,
                                   void* opaque)
{
    XPR_Timer* timer = (XPR_Timer*)nodeAlloc();
    if (timer) {
        timer->id = id;
        timer->interval = interval;
        timer->fn = fn;
        timer->opaque = opaque;
        timer->flags |= XPR_TIMER_FLAG_ENABLED;
    }
    return timer;
}

XPR_API void XPR_TimerDestroy(XPR_Timer* self)
{
    nodeFree(self);
}

XPR_API void XPR_TimerEnable(XPR_Timer* self)
{
    if (self)
        self->flags |= XPR_TIMER_FLAG_ENABLED;
}

XPR_API void XPR_TimerDisable(XPR_Timer* self)
{
    if (self)
        self->flags &= ~XPR_TIMER_FLAG_ENABLED;
}

XPR_API void XPR_TimerSetInterval(XPR_Timer* self, int64_t interval)
{
    if (self)
        self->interval = interval;
}

static int64_t timespecInUs(void)
{
    int64_t us = 0;
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    us = ts.tv_sec;
    us *= 1000000;
    us += ts.tv_nsec / 1000;
    return us;
}

// Run timer action
static inline XPR_TimerReturn timerExec(XPR_Timer* timer)
{
    XPR_TimerReturn ret = timer->fn(timer->opaque);
    if (ret == XPR_TIMER_CONTINUE)
        timer->expire = timespecInUs() + timer->interval;
    return ret;
}

static void* timerQueueLoop(void* opaque, XPR_Thread* thread)
{
    XPR_TimerQueue* self = (XPR_TimerQueue*)(opaque);
    XPR_Timer* timer = NULL;
    struct timespec ts;
    /* We allow thread cancellation at any time */
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    while (!self->exitLoop) {
        pthread_mutex_lock(&self->lock);
        /* Get first event */
        timer = XPR_ListFirstNl(self->activeTimers);
        /*
         * If we have timers in queue, we setup a timer to wait for first one.
         * In all cases, thread is woken up when a reschedule occurs.
         */
        if (timer) {
            ts.tv_sec = timer->expire / 1000000;
            ts.tv_nsec = (timer->expire % 1000000) * 1000;
            pthread_cond_timedwait(&self->schedule, &self->lock, &ts);
        }
        else {
            /* We just wait for reschedule since we don't have any timer */
            pthread_cond_wait(&self->schedule, &self->lock);
        }
        /* We need to check "running" flags to know if we must stop */
        if (self->exitLoop) {
            pthread_mutex_unlock(&self->lock);
            break;
        }
        /*
         * Now, we need to find why we were woken up. So, we compare current
         * time with first timer to see if we must execute action associated
         * with it.
         */
        int64_t now = timespecInUs();
        /* Get first event */
        timer = XPR_ListFirstNl(self->activeTimers);
        /* If there is nothing to do for now, wait again */
        if ((timer == NULL) || (timer->expire > now)) {
            pthread_mutex_unlock(&self->lock);
            continue;
        }
        /*
         * We have a timer to manage. Remove it from queue and mark it as
         * running.
         */
        XPR_ListTakeNl(self->activeTimers, timer);
        /* Execute user function and reschedule timer if required */
        XPR_TimerReturn tr = timerExec(timer);
        if (tr == XPR_TIMER_CONTINUE)
            XPR_ListAppend(self->activeTimers, timer);
        else if (tr == XPR_TIMER_SUSPEND)
            XPR_ListAppend(self->suspendTimers, timer);
        else
            nodeFree(timer);
        pthread_mutex_unlock(&self->lock);
    }
    return NULL;
}

XPR_API XPR_TimerQueue* XPR_TimerQueueCreate(void)
{
    XPR_TimerQueue* self = XPR_Alloc(sizeof(*self));
    if (self) {
        self->activeTimers = XPR_ListCreate(XPR_LIST_SINGLY_LINKED, nodeAlloc,
                                            nodeFree, nodeCompare);
        self->suspendTimers = XPR_ListCreate(XPR_LIST_SINGLY_LINKED, nodeAlloc,
                                             nodeFree, nodeCompare);
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&self->lock, &attr);
        pthread_cond_init(&self->schedule, NULL);
        self->thread = XPR_ThreadCreate(timerQueueLoop, 0, self);
    }
    return self;
}

XPR_API void XPR_TimerQueueDestroy(XPR_TimerQueue* self)
{
    if (self) {
        pthread_mutex_lock(&self->lock);
        self->exitLoop = 1;
        pthread_cond_signal(&self->schedule);
        XPR_ThreadJoin(self->thread);
        XPR_ThreadDestroy(self->thread);
        XPR_ListDestroy(self->activeTimers);
        XPR_ListDestroy(self->suspendTimers);
        pthread_mutex_unlock(&self->lock);
        pthread_mutex_destroy(&self->lock);
        pthread_cond_destroy(&self->schedule);
    }
}

XPR_API int XPR_TimerQueueAdd(XPR_TimerQueue* self, XPR_Timer* timer)
{
    if (!self || !timer)
        return XPR_ERR_GEN_NULL_PTR;
    int err = XPR_ERR_OK;
    pthread_mutex_lock(&self->lock);
    if (timer->flags & XPR_TIMER_FLAG_ENABLED)
        err = XPR_ListAppendNl(self->activeTimers, timer);
    else
        err = XPR_ListAppendNl(self->suspendTimers, timer);
    pthread_mutex_unlock(&self->lock);
    return err;
}

XPR_API XPR_Timer* XPR_TimerQueueFind(XPR_TimerQueue* self, XPR_TimerId id)
{
    XPR_Timer* timer = NULL;
    pthread_mutex_lock(&self->lock);
    timer = XPR_ListFirstNl(self->activeTimers);
    while (timer->id != id) {
        timer = XPR_ListNextNl(self->activeTimers, timer);
    }
    if (timer == NULL) {
        timer = XPR_ListFirstNl(self->suspendTimers);
        while (timer->id != id) {
            timer = XPR_ListNextNl(self->suspendTimers, timer);
        }
    }
    pthread_mutex_unlock(&self->lock);
    return timer;
}

XPR_API int XPR_TimerQueueRemove(XPR_TimerQueue* self, XPR_Timer* timer)
{
    int err = XPR_ERR_OK;
    pthread_mutex_lock(&self->lock);
    err = XPR_ListRemoveNl(self->activeTimers, timer);
    if (err != XPR_ERR_OK)
        err = XPR_ListRemoveNl(self->suspendTimers, timer);
    pthread_mutex_unlock(&self->lock);
    return err;
}
