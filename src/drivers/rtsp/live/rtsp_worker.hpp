#ifndef XPR_RTSP_DRIVER_LIVE_RTSP_WORKER_HPP
#define XPR_RTSP_DRIVER_LIVE_RTSP_WORKER_HPP

#include <stdint.h>
#include <string>
#include <live/BasicUsageEnvironment.hh>
#include <live/FramedSource.hh>
#include <live/liveMedia.hh>
#include <xpr/xpr_fifo.h>
#include "rtsp.hpp"

namespace xpr
{
namespace rtsp
{

struct TaskData
{
    int port;
    TaskId task;
};

class Worker
{
public:
    Worker(int id, Port* parent);
    virtual ~Worker(void);

    // Methods
    void run(void);
    int start(void);
    int stop(void);
    int terminate(void);

    // Push an async task to the fifo
    int asyncTask(int port, TaskId task);

    // Properties

    int id(void) const;

    Port* parent(void);

    TaskScheduler& scheduler(void);

    UsageEnvironment& env(void);

private:

    static void* thread(void* opaque, XPR_Thread* thread);

private:
    int                 mId;
    Port*               mParent;
    TaskScheduler*      mScheduler;
    UsageEnvironment*   mEnv;
    XPR_Fifo*           mAsyncTasks;
    XPR_Thread*         mThread;
    bool                mExitLoop;
};

} // namespace xpr::rtsp
} // namespace xpr

#endif // XPR_RTSP_DRIVER_LIVE_RTSP_WORKER_HPP