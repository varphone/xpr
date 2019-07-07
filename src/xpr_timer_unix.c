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
#include <xpr/xpr_atomic.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_list.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_sync.h>
#include <xpr/xpr_sys.h>
#include <xpr/xpr_thread.h>
#include <xpr/xpr_timer.h>
#include <xpr/xpr_utils.h>

#define XPR_TIMER_FLAG_ENABLED 0x0001
#define XPR_TIMER_FLAG_SINGLESHOT 0x0002
#define XPR_TIMER_FLAG_TERMINATE 0x0004
#define XPR_TIMER_TS_UNIT 1000000

struct XPR_Timer
{
    XPR_ListNodeSL _base;   // SinglyLinked list node base
    XPR_TimerId id;         // Timer identifer
    int64_t interval;       // Interval in us
    XPR_TimerFn fn;         // Callback on expired
    void* opaque;           // User context data for callback
    int64_t expire;         // Expire timestamp
    int flags;              // Flags to control the timer
    XPR_TimerQueue* timerQ; // TimerQueue reference
};

struct XPR_TimerQueue
{
    int exitLoop;            // Exit thread loop
    XPR_List* activeTimers;  // Actived timers
    XPR_List* suspendTimers; // Suspended timers
    pthread_mutex_t lock;    // Resource lock
    pthread_cond_t cond;     // Schedule cond
    XPR_Thread* thread;      // Thread object
};

// Default timer queue.
static XPR_TimerQueue* sDefaultQueue = NULL;
static pthread_mutex_t sDefaultQueueLock =
    PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

#define DEFAULT_QUEUE_LOCK() pthread_mutex_lock(&sDefaultQueueLock)
#define DEFAULT_QUEUE_UNLOCK() pthread_mutex_unlock(&sDefaultQueueLock)

XPR_API XPR_TimerId XPR_TimerIdNew()
{
    static XPR_Atomic sTimerIdSeq = 0;
    return (XPR_TimerId)(XPR_AtomicInc(&sTimerIdSeq));
}

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
        return 1;
    return 0;
}

XPR_API XPR_Timer* XPR_TimerCreate(XPR_TimerId id, int64_t interval,
                                   XPR_TimerFn fn, void* opaque)
{
    XPR_Timer* timer = (XPR_Timer*)nodeAlloc();
    if (timer) {
        timer->id = id;
        timer->interval = interval;
        timer->fn = fn;
        timer->opaque = opaque;
        timer->flags = 0;
        timer->timerQ = NULL;
    }
    return timer;
}

XPR_API void XPR_TimerDestroy(XPR_Timer* self)
{
    if (self) {
        int err = XPR_ERR_ERROR;
        if (self->timerQ)
            err = XPR_TimerQueueRemove(self->timerQ, self);
        if (err != XPR_ERR_OK)
            nodeFree(self);
    }
}

XPR_API void XPR_TimerEnable(XPR_Timer* self)
{
    if (self && !(self->flags & XPR_TIMER_FLAG_ENABLED)) {
        if (self->timerQ)
            XPR_TimerQueueResume(self->timerQ, self);
        else
            self->flags |= XPR_TIMER_FLAG_ENABLED;
    }
}

XPR_API void XPR_TimerDisable(XPR_Timer* self)
{
    if (self && (self->flags & XPR_TIMER_FLAG_ENABLED)) {
        if (self->timerQ)
            XPR_TimerQueueSuspend(self->timerQ, self);
        else
            self->flags &= ~XPR_TIMER_FLAG_ENABLED;
    }
}

XPR_API XPR_TimerId XPR_TimerGetId(XPR_Timer* self)
{
    if (!self)
        return 0;
    return self->id;
}

XPR_API int64_t XPR_TimerGetInterval(XPR_Timer* self)
{
    if (!self)
        return 0;
    return self->interval;
}

XPR_API int XPR_TimerIsEnabled(XPR_Timer* self)
{
    if (!self)
        return XPR_ERR_GEN_NULL_PTR;
    return (self->flags & XPR_TIMER_FLAG_ENABLED) ? XPR_TRUE : XPR_FALSE;
}

XPR_API void XPR_TimerSetInterval(XPR_Timer* self, int64_t interval)
{
    if (self)
        self->interval = interval;
}

XPR_API void XPR_TimerTerminate(XPR_Timer* self)
{
    if (self)
        self->flags |= XPR_TIMER_FLAG_TERMINATE;
}

static inline XPR_TimerReturn timerExec(XPR_Timer* timer)
{
    if (timer->flags & XPR_TIMER_FLAG_TERMINATE)
        return XPR_TIMER_TERMINATE;
    XPR_TimerReturn ret = timer->fn(timer->opaque);
    if (ret == XPR_TIMER_CONTINUE) {
        // Reset the expire
        timer->expire = XPR_SYS_GetCTS() + timer->interval;
    }
    if (timer->flags & XPR_TIMER_FLAG_SINGLESHOT)
        ret = XPR_TIMER_TERMINATE;
    return ret;
}

static void* timerQueueLoop(void* opaque, XPR_Thread* thread)
{
    XPR_TimerQueue* self = (XPR_TimerQueue*)(opaque);
    XPR_Timer* timer = NULL;
    XPR_Timer* nextTimer = NULL;
    // We allow thread cancellation at any time
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    while (!self->exitLoop) {
        pthread_mutex_lock(&self->lock);
        timer = XPR_ListFirstNl(self->activeTimers);
        // If timer was setted, wait to cond
        if (timer) {
            struct timespec ts;
            ts.tv_sec = timer->expire / XPR_TIMER_TS_UNIT;
            ts.tv_nsec = (timer->expire % XPR_TIMER_TS_UNIT) * 1000;
            pthread_cond_timedwait(&self->cond, &self->lock, &ts);
        }
        else {
            // We just wait for recond since we don't have any timer
            pthread_cond_wait(&self->cond, &self->lock);
        }
        // We need to check "running" flags to know if we must stop
        if (self->exitLoop) {
            pthread_mutex_unlock(&self->lock);
            break;
        }
        // Inspect all expired timers
        timer = XPR_ListFirstNl(self->activeTimers);
        while (timer) {
            if (timer->expire > XPR_SYS_GetCTS())
                break;
            nextTimer = XPR_ListNextNl(self->activeTimers, timer);
            XPR_ListTakeNl(self->activeTimers, timer);
            if (timer->flags & XPR_TIMER_FLAG_ENABLED) {
                XPR_TimerReturn tr = timerExec(timer);
                if (tr == XPR_TIMER_CONTINUE)
                    XPR_ListAppend(self->activeTimers, timer);
                else if (tr == XPR_TIMER_SUSPEND)
                    XPR_ListAppend(self->suspendTimers, timer);
                else
                    XPR_ListFree(self->activeTimers, timer);
            }
            else {
                XPR_ListAppend(self->suspendTimers, timer);
            }
            timer = nextTimer;
        }
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
        pthread_mutexattr_t mutexAttr;
        pthread_mutexattr_init(&mutexAttr);
        pthread_mutexattr_settype(&mutexAttr, PTHREAD_MUTEX_RECURSIVE);
        pthread_mutex_init(&self->lock, &mutexAttr);
        pthread_condattr_t condAttr;
        pthread_condattr_init(&condAttr);
        pthread_condattr_setclock(&condAttr, CLOCK_MONOTONIC);
        pthread_cond_init(&self->cond, &condAttr);
        self->thread = XPR_ThreadCreate(timerQueueLoop, 0, self);
    }
    return self;
}

XPR_API void XPR_TimerQueueDestroy(XPR_TimerQueue* self)
{
    if (self) {
        // Signal the thread exit
        pthread_mutex_lock(&self->lock);
        self->exitLoop = 1;
        pthread_cond_signal(&self->cond);
        pthread_mutex_unlock(&self->lock);
        // Wait for thread exited
        XPR_ThreadJoin(self->thread);
        XPR_ThreadDestroy(self->thread);
        // Destroy all timers
        pthread_mutex_lock(&self->lock);
        XPR_ListDestroy(self->activeTimers);
        XPR_ListDestroy(self->suspendTimers);
        pthread_mutex_unlock(&self->lock);
        // Destroy the lock and cond
        pthread_mutex_destroy(&self->lock);
        pthread_cond_destroy(&self->cond);
        // Destroy self
        XPR_Free(self);
    }
}

XPR_API XPR_TimerQueue* XPR_TimerQueueDefault(void)
{
    DEFAULT_QUEUE_LOCK();
    if (sDefaultQueue == NULL) {
        sDefaultQueue = XPR_TimerQueueCreate();
    }
    DEFAULT_QUEUE_UNLOCK();
    return sDefaultQueue;
}

XPR_API int XPR_TimerQueueAdd(XPR_TimerQueue* self, XPR_Timer* timer)
{
    if (!self || !timer)
        return XPR_ERR_GEN_NULL_PTR;
    int err = XPR_ERR_OK;
    pthread_mutex_lock(&self->lock);
    timer->timerQ = self;
    timer->expire = XPR_SYS_GetCTS() + timer->interval;
    if (timer->flags & XPR_TIMER_FLAG_ENABLED)
        err = XPR_ListAppendNl(self->activeTimers, timer);
    else
        err = XPR_ListAppendNl(self->suspendTimers, timer);
    if (err == XPR_ERR_OK)
        pthread_cond_signal(&self->cond);
    pthread_mutex_unlock(&self->lock);
    return err;
}

XPR_API int XPR_TimerQueueAddNew(XPR_TimerQueue* self, XPR_TimerId id,
                                 int64_t interval, XPR_TimerFn fn, void* opaque)
{
    int err = XPR_ERR_ERROR;
    XPR_Timer* timer = XPR_TimerCreate(id, interval, fn, opaque);
    if (!timer)
        return XPR_ERR_GEN_NOMEM;
    return XPR_TimerQueueAdd(self, timer);
}

XPR_API XPR_Timer* XPR_TimerQueueFindId(XPR_TimerQueue* self, XPR_TimerId id)
{
    if (!self || id == XPR_TIMER_ID_SINGLESHOT)
        return NULL;
    XPR_Timer* timer = NULL;
    pthread_mutex_lock(&self->lock);
    timer = XPR_ListFirstNl(self->activeTimers);
    while (timer && timer->id != id) {
        timer = XPR_ListNextNl(self->activeTimers, timer);
    }
    if (timer == NULL) {
        timer = XPR_ListFirstNl(self->suspendTimers);
        while (timer && timer->id != id) {
            timer = XPR_ListNextNl(self->suspendTimers, timer);
        }
    }
    // Don't exposure the timer if marked as TERMINATE
    if (timer && timer->flags & XPR_TIMER_FLAG_TERMINATE)
        timer = NULL;
    pthread_mutex_unlock(&self->lock);
    return timer;
}

XPR_API int XPR_TimerQueueRemove(XPR_TimerQueue* self, XPR_Timer* timer)
{
    if (!self || !timer)
        return XPR_ERR_GEN_NULL_PTR;
    int err = XPR_ERR_OK;
    pthread_mutex_lock(&self->lock);
    err = XPR_ListRemoveNl(self->activeTimers, timer);
    if (err != XPR_ERR_OK)
        err = XPR_ListRemoveNl(self->suspendTimers, timer);
    if (err == XPR_ERR_OK)
        pthread_cond_signal(&self->cond);
    pthread_mutex_unlock(&self->lock);
    return err;
}

XPR_API int XPR_TimerQueueRemoveId(XPR_TimerQueue* self, XPR_TimerId id)
{
    int err = XPR_ERR_ERROR;
    XPR_Timer* timer = XPR_TimerQueueFindId(self, id);
    if (!timer)
        return XPR_ERR_GEN_UNEXIST;
    return XPR_TimerQueueRemove(self, timer);
}

XPR_API int XPR_TimerQueueResume(XPR_TimerQueue* self, XPR_Timer* timer)
{
    if (!self || !timer)
        return XPR_ERR_GEN_NULL_PTR;
    int err = XPR_ERR_OK;
    pthread_mutex_lock(&self->lock);
    err = XPR_ListTakeNl(self->suspendTimers, timer);
    if (err == XPR_ERR_OK) {
        timer->flags |= XPR_TIMER_FLAG_ENABLED;
        XPR_ListAppendNl(self->activeTimers, timer);
        pthread_cond_signal(&self->cond);
    }
    pthread_mutex_unlock(&self->lock);
    return err;
}

XPR_API int XPR_TimerQueueResumeId(XPR_TimerQueue* self, XPR_TimerId id)
{
    int err = XPR_ERR_ERROR;
    XPR_Timer* timer = XPR_TimerQueueFindId(self, id);
    if (!timer)
        return XPR_ERR_GEN_UNEXIST;
    return XPR_TimerQueueResume(self, timer);
}

XPR_API int XPR_TimerQueueSingleShot(XPR_TimerQueue* self, int64_t interval,
                                     XPR_TimerFn fn, void* opaque)
{
    if (!self || !fn)
        return XPR_ERR_GEN_NULL_PTR;
    int err = XPR_ERR_ERROR;
    XPR_Timer* timer =
        XPR_TimerCreate(XPR_TIMER_ID_SINGLESHOT, interval, fn, opaque);
    if (timer) {
        timer->flags |= XPR_TIMER_FLAG_ENABLED | XPR_TIMER_FLAG_SINGLESHOT;
        err = XPR_TimerQueueAdd(self, timer);
        if (err != XPR_ERR_OK)
            XPR_TimerDestroy(timer);
    }
    return err;
}

XPR_API int XPR_TimerQueueSuspend(XPR_TimerQueue* self, XPR_Timer* timer)
{
    if (!self || !timer)
        return XPR_ERR_GEN_NULL_PTR;
    int err = XPR_ERR_OK;
    pthread_mutex_lock(&self->lock);
    err = XPR_ListTakeNl(self->activeTimers, timer);
    if (err == XPR_ERR_OK) {
        timer->flags &= ~XPR_TIMER_FLAG_ENABLED;
        XPR_ListAppendNl(self->suspendTimers, timer);
        pthread_cond_signal(&self->cond);
    }
    pthread_mutex_unlock(&self->lock);
    return err;
}

XPR_API int XPR_TimerQueueSuspendId(XPR_TimerQueue* self, XPR_TimerId id)
{
    int err = XPR_ERR_ERROR;
    XPR_Timer* timer = XPR_TimerQueueFindId(self, id);
    if (!timer)
        return XPR_ERR_GEN_UNEXIST;
    return XPR_TimerQueueSuspend(self, timer);
}
