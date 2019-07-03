#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_list.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_sys.h>
#include <xpr/xpr_timer.h>
#include <xpr/xpr_thread.h>
#include <xpr/xpr_utils.h>

#define kMaxTimers 100

static XPR_Timer* sTimers[kMaxTimers];
static XPR_TimerQueue* sTimerQ = NULL;

static inline int64_t randInterval(void)
{
    return (rand() % (100 * XPR_SYS_CTS_UNIT)) + (XPR_SYS_CTS_UNIT / 10);
}

static XPR_TimerReturn timerExpired(void* opaque)
{
    DBG(DBG_L3, "Timer[%d] expired", (uintptr_t)(opaque));
    return XPR_TIMER_CONTINUE;
}

static XPR_TimerReturn singleShotExpired(void* opaque)
{
    DBG(DBG_L3, "Single Shot Timer (%d) expired", (uintptr_t)(opaque));
    return XPR_TIMER_CONTINUE;
}

#if 0
static int XPR_SYS_WaitKeys(int timeout)
{
    fd_set rfds;
    struct timeval tv;
    FD_ZERO(&rfds);
    tv.tv_sec = timeout / kTimeUnit;
    tv.tv_usec = timeout % kTimeUnit;
    FD_SET(STDIN_FILENO, &rfds);
    int n = select(STDIN_FILENO + 1, &rfds, NULL, NULL, &tv);
    return n == 0 ? 0 : 1;
}
#endif

void benchmark(void)
{
    srand(XPR_SYS_GetCTS());
    sTimerQ = XPR_TimerQueueCreate();
    for (int i = 0; i < kMaxTimers; i++) {
        sTimers[i] = XPR_TimerCreate(i, randInterval(), timerExpired,
                                     (void*)(uintptr_t)(i));
        XPR_TimerEnable(sTimers[i]);
        if (XPR_TimerQueueAdd(sTimerQ, sTimers[i]) == XPR_ERR_OK) {
            DBG(DBG_L3, "Timer[%d @ %p] interval=%ld added", i, sTimers[i],
                XPR_TimerGetInterval(sTimers[i]));
        }
    }

    XPR_TimerQueueSingleShot(sTimerQ, 1 * XPR_SYS_CTS_UNIT, singleShotExpired,
                             (void*)(1 * XPR_SYS_CTS_UNIT));
    XPR_TimerQueueSingleShot(sTimerQ, 2 * XPR_SYS_CTS_UNIT, singleShotExpired,
                             (void*)(2 * XPR_SYS_CTS_UNIT));
    XPR_TimerQueueSingleShot(sTimerQ, 3 * XPR_SYS_CTS_UNIT, singleShotExpired,
                             (void*)(3 * XPR_SYS_CTS_UNIT));
    XPR_TimerQueueSingleShot(sTimerQ, 4 * XPR_SYS_CTS_UNIT, singleShotExpired,
                             (void*)(4 * XPR_SYS_CTS_UNIT));
    XPR_TimerQueueSingleShot(sTimerQ, 5 * XPR_SYS_CTS_UNIT, singleShotExpired,
                             (void*)(5 * XPR_SYS_CTS_UNIT));

    DBG(DBG_L1, "Press any key to exit ...");

    // Wait for key press timeout for random operations
    while (1) {
        int tmo = rand() % (1 * XPR_SYS_CTS_UNIT);
        if (XPR_SYS_WaitKey(tmo))
            break;
        // Random find a timer
        XPR_Timer* timer = XPR_TimerQueueFindId(sTimerQ, rand() % kMaxTimers);
        if (!timer)
            continue;
        // Roll operations
        int r = rand() % 100;
        if (r > 40 && r < 60) {
            XPR_TimerDisable(timer);
            DBG(DBG_L1, "Timer[%d] disabled", XPR_TimerGetId(timer));
        }
        else if (r > 20 && r < 40) {
            XPR_TimerTerminate(timer);
            DBG(DBG_L1, "Timer[%d] terminated", XPR_TimerGetId(timer));
        }
        else if (r > 80) {
            XPR_TimerEnable(timer);
            DBG(DBG_L1, "Timer[%d] enabled", XPR_TimerGetId(timer));
        }
    }

    // Destroy timer queue and all timers in the queue
    XPR_TimerQueueDestroy(sTimerQ);
}
