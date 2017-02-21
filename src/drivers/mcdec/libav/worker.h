#ifndef XPR_MCDEC_WORKER_H
#define XPR_MCDEC_WORKER_H

#include <stdint.h>
#include <xpr/xpr_atomic.h>
#include <xpr/xpr_fifo.h>
#include <xpr/xpr_thread.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XPR_MCDEC_WORKER_MAX_TASKS  2048

// Forwards
#ifndef XPR_MCDEC_RUNTIME_TYPE_DEFINED
#define XPR_MCDEC_RUNTIME_TYPE_DEFINED
struct XPR_MCDEC_Runtime;
typedef struct XPR_MCDEC_Runtime XPR_MCDEC_Runtime;
#endif //XPR_MCDEC_RUNTIME_TYPE_DEFINED

#ifndef XPR_MCDEC_WORKER_TYPE_DEFINED
#define XPR_MCDEC_WORKER_TYPE_DEFINED
struct XPR_MCDEC_Worker {
    int index;              ///< Worker index
    XPR_Atomic exitLoop;    ///< Loop condition
    XPR_Fifo* taskQueue;    ///< Task queue
    XPR_Thread* thread;     ///< Worker thread
};
typedef struct XPR_MCDEC_Worker XPR_MCDEC_Worker;
#endif // XPR_MCDEC_WORKER_TYPE_DEFINED

XPR_MCDEC_Worker* XPR_MCDEC_WorkerNew(XPR_MCDEC_Runtime* rt, int maskTasks);
int XPR_MCDEC_WorkerDestroy(XPR_MCDEC_Worker* worker);

#ifdef __cplusplus
}
#endif

#endif // XPR_MCDEC_WORKER_H
