#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_mem.h>
#include "runtime.h"
#include "worker.h"

XPR_MCDEC_Runtime* XPR_MCDEC_RuntimeNew(int maxWorkers)
{
    int i = 0;
    XPR_MCDEC_Runtime* rt = (XPR_MCDEC_Runtime*)XPR_Alloc(sizeof(*rt));
    if (rt) {
        memset(rt, 0, sizeof(*rt));
        rt->maxWorkers = maxWorkers;
        for (; i < maxWorkers; i++) {
            rt->workers[i] = XPR_MCDEC_WorkerNew(rt, XPR_MCDEC_WORKER_MAX_TASKS);
            if (!rt->workers[i])
                goto fail;
            rt->workers[i]->index = i;
        }
    }
    return rt;
fail:
    XPR_MCDEC_RuntimeDestroy(rt);
    return 0;
}

int XPR_MCDEC_RuntimeDestroy(XPR_MCDEC_Runtime* rt)
{
    return XPR_ERR_OK;
}

XPR_MCDEC_Worker* XPR_MCDEC_RuntimeGetWorker(const XPR_MCDEC_Runtime* rt, int index)
{
    return rt->workers[index % rt->maxWorkers];
}

XPR_StreamBlock* XPR_MCDEC_RuntimeGetCSB(XPR_MCDEC_Runtime* rt, int size)
{
    int i = 0;
    XPR_Fifo* f = 0;
    XPR_StreamBlock* sb = 0;
    if (size > 1024*1024)
        return XPR_StreamBlockAlloc(size);
    i = size / 16 / 2;
    if (i >= XPR_MCDEC_MAX_CSB_QUEUES)
        return 0;
    f = rt->csbQueues[i];
    sb = (XPR_StreamBlock*)XPR_FifoGetAsAtomic(f);
    if (!sb)
        sb = XPR_StreamBlockAlloc(size);
    return sb;
}

int XPR_MCDEC_RuntimePutCSB(XPR_MCDEC_Runtime* rt, XPR_StreamBlock* sb)
{
    int i = 0;
    XPR_Fifo* f = 0;
    i = (int)XPR_StreamBlockSize(sb) / 16 / 2;
    if (i >= XPR_MCDEC_MAX_CSB_QUEUES)
        return XPR_ERR_ERROR;
    f = rt->csbQueues[i];
    XPR_FifoPutAsAtomic(f, (uintptr_t)sb);
    return XPR_ERR_OK;
}