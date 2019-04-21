#include "rtsp_worker.hpp"
#include <sstream>

namespace xpr
{

namespace rtsp
{

class MyBasicUsageEnvironment : public BasicUsageEnvironment {
public:
    virtual UsageEnvironment& operator<<(char const* str)
    {
        return *this;
    }

    virtual UsageEnvironment& operator<<(int i)
    {
        return *this;
    }

    virtual UsageEnvironment& operator<<(unsigned u)
    {
        return *this;
    }

    virtual UsageEnvironment& operator<<(double d)
    {
        return *this;
    }

    virtual UsageEnvironment& operator<<(void* p)
    {
        return *this;
    }

    static MyBasicUsageEnvironment* createNew(TaskScheduler& scheduler)
    {
        return new MyBasicUsageEnvironment(scheduler);
    }

protected:
    explicit MyBasicUsageEnvironment(TaskScheduler& scheduler)
        : BasicUsageEnvironment(scheduler)
    {
    }

private:
    std::stringstream mStream;
};

// Worker
//============================================================================
Worker::Worker(int id, Port* parent)
    : mId(id)
    , mParent(parent)
    , mScheduler(NULL)
    , mEnv(NULL)
    , mAsyncTasks(NULL)
    , mThread(NULL)
    , mExitLoop(false)
{
    DBG(DBG_L5, "XPR_RTSP: Worker::Worker(%d, %p) = %p", id, parent, this);
    mScheduler = BasicTaskScheduler::createNew();
    mEnv = MyBasicUsageEnvironment::createNew(*mScheduler);
    mAsyncTasks = XPR_FifoCreate(sizeof(TaskData), 128);
}

Worker::~Worker(void)
{
    DBG(DBG_L5, "XPR_RTSP: Worker::~Worker(%d, %p) = %p", mId, mParent, this);
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
    if (mAsyncTasks) {
        XPR_FifoDestroy(mAsyncTasks);
        mAsyncTasks = NULL;
    }
}

void Worker::run(void)
{
    DBG(DBG_L4, "XPR_RTSP: Worker[%d @ %p]: running ...", id(), this);
    while (!mExitLoop) {
        while (!XPR_FifoIsEmpty(mAsyncTasks)) {
            TaskData td = {0, XPR_RTSP_TASK_NULL};
            if (XPR_FifoGet(mAsyncTasks, &td, 1) < 0)
                break;
            mParent->runTask(td.port, td.task);
        }
        ((BasicTaskScheduler*)mScheduler)->SingleStep(0);
    }
    DBG(DBG_L4, "XPR_RTSP: Worker[%d @ %p]: exited.", id(), this);
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

int Worker::asyncTask(int port, TaskId task)
{
    TaskData td = {port, task};
    int ret = XPR_FifoPut(mAsyncTasks, &td, 1);
    return ret > 0 ? XPR_ERR_OK : ret;
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

