#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_mem.h>
#include "runtime.h"
#include "worker.h"

static void* ThreadRoutine(XPR_Thread* thread, void* opaque)
{
    XPR_MCDEC_Worker* w = (XPR_MCDEC_Worker*)opaque;
    while (!w->exitLoop) {
        printf("AAAAA\n");
        XPR_ThreadSleep(1000000);
    }
    return (void*)0;
}

XPR_MCDEC_Worker* XPR_MCDEC_WorkerNew(XPR_MCDEC_Runtime* rt, int maxTasks)
{
    XPR_MCDEC_Worker* w = (XPR_MCDEC_Worker*)XPR_Alloc(sizeof(*w));
    if (w) {
        memset(w, 0, sizeof(*w));
        w->taskQueue = XPR_FifoCreate(sizeof(uintptr_t), maxTasks);
        w->thread = XPR_ThreadCreate(ThreadRoutine, 0, w);
        if (!w->taskQueue || !w->thread)
            goto fail;
    }
    return w;
fail:
    XPR_MCDEC_WorkerDestroy(w);
    return 0;
}

int XPR_MCDEC_WorkerDestroy(XPR_MCDEC_Worker* worker)
{
    if (worker) {
        if (worker->thread) {
            XPR_AtomicAssign(&worker->exitLoop, 0);
            XPR_ThreadJoin(worker->thread);
            XPR_ThreadDestroy(worker->thread);
            worker->thread = 0;
        }
        if (worker->taskQueue) {
            XPR_FifoDestroy(worker->taskQueue);
            worker->taskQueue = 0;
        }
        XPR_Free(worker);
    }
    return XPR_ERR_OK;
}
