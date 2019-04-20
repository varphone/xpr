#include "rtsp_worker.hpp"

namespace xpr
{

namespace rtsp
{

// Worker
//============================================================================
Worker::Worker(int id, Port* parent)
    : mId(id)
    , mParent(parent)
    , mScheduler(NULL)
    , mEnv(NULL)
    , mThread(NULL)
    , mExitLoop(false)
{
    DBG(DBG_L4, "Worker::Worker(%d, %p) = %p", id, parent, this);
    mScheduler = BasicTaskScheduler::createNew();
    mEnv = BasicUsageEnvironment::createNew(*mScheduler);
}

Worker::~Worker(void)
{
    DBG(DBG_L4, "Worker::~Worker(%d, %p) = %p", mId, mParent, this);
    stop();
    //
    if (mEnv != NULL) {
        mEnv->reclaim();
        mEnv = NULL;
    }
    if (mScheduler != NULL) {
        delete mScheduler;
        mScheduler = NULL;
    }
}

void Worker::run(void)
{
    DBG(DBG_L4, "worker [%d @ %p] running ...", id(), this);
    while (!mExitLoop) {
        ((BasicTaskScheduler*)mScheduler)->SingleStep(0);
    }
    DBG(DBG_L4, "worker [%d @ %p] exited.", id(), this);
}

int Worker::start(void)
{
    if (mScheduler == NULL || mEnv == NULL)
        return XPR_ERR_GEN_SYS_NOTREADY;
    if (mThread != NULL)
        return XPR_ERR_GEN_BUSY;
    mThread = XPR_ThreadCreate(Worker::thread, 1024 * 1024 * 1, this);
    return XPR_ERR_OK;
}

int Worker::stop(void)
{
    if (mThread == NULL)
        return XPR_ERR_GEN_SYS_NOTREADY;
    if (mExitLoop != true)
        mExitLoop = true;
    if (mThread != NULL) {
        XPR_ThreadJoin(mThread);
        XPR_ThreadDestroy(mThread);
        mThread = NULL;
    }
    return XPR_ERR_OK;
}

int Worker::terminate(void)
{
    mExitLoop = true;
    return XPR_ERR_OK;
}

int Worker::id(void) const
{
    return mId;
}

Port* Worker::parent(void)
{
    return mParent;
}

TaskScheduler& Worker::scheduler(void)
{
    return *mScheduler;
}

UsageEnvironment& Worker::env(void)
{
    return *mEnv;
}

void* Worker::thread(void* opaque, XPR_Thread* thread)
{
    if (opaque)
        ((Worker*)opaque)->run();
    return NULL;
}

} // namespace xpr::rtsp

} // namespace xpr

