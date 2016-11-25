#ifndef XPR_MCDEC_RUNTIME_H
#define XPR_MCDEC_RUNTIME_H

#include <stdint.h>
#include <xpr/xpr_fifo.h>
#include <xpr/xpr_streamblock.h>
#include "task.h"
#include "worker.h"

#ifdef __cplusplus
extern "C" {
#endif

#define XPR_MCDEC_MAX_WORKERS       8
#define XPR_MCDEC_MAX_CSB_QUEUES    34

typedef struct XPR_MCDEC_Runtime {
    int maxWorkers;
    XPR_MCDEC_Worker* workers[XPR_MCDEC_MAX_WORKERS];
    XPR_Fifo* csbQueues[XPR_MCDEC_MAX_CSB_QUEUES];
} XPR_MCDEC_Runtime;

XPR_MCDEC_Runtime* XPR_MCDEC_RuntimeNew(int maxWorkers);
int XPR_MCDEC_RuntimeDestroy(XPR_MCDEC_Runtime* rt);

XPR_MCDEC_Worker* XPR_MCDEC_RuntimeGetWorker(const XPR_MCDEC_Runtime* rt, int refIndex);

///
/// Get cached stream block
/// @param [in] rt      Runtime
/// @param [in] size    Stream block size
XPR_StreamBlock* XPR_MCDEC_RuntimeGetCSB(XPR_MCDEC_Runtime* rt, int size);

#ifdef __cplusplus
}
#endif

#endif // XPR_MCDEC_RUNTIME_H
