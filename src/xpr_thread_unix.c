#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <xpr/xpr_thread.h>

struct XPR_Thread {
    pthread_t handle;
    XPR_ThreadStartRoutine startRoutine;
    XPR_ThreadEndRoutine endRoutine;
    unsigned int flags;
    void* opaque;
    void* userData;
};

static void* XPR_ThreadRoutineWrapper(void *opaque)
{
    XPR_Thread* thread = (XPR_Thread*)opaque;
    void* result = thread->startRoutine(thread->opaque, thread);
    if (thread->endRoutine)
        thread->endRoutine(thread->opaque, thread);
    return result;
}

XPR_Thread* XPR_ThreadCreate(XPR_ThreadStartRoutine routine, unsigned int stackSize, void* opaque)
{
    XPR_Thread* t = (XPR_Thread*)calloc(sizeof(*t), 1);
    if (t) {
        t->handle = 0;
        t->startRoutine = routine;
        t->endRoutine = 0;
        t->flags = 0;
        t->opaque = opaque;
        t->userData = 0;
        if (pthread_create(&t->handle, 0, XPR_ThreadRoutineWrapper, t) < 0) {
            free((void*)t);
            t = 0;
        }
    }
    return t;
}

XPR_Thread* XPR_ThreadCreateEx(XPR_ThreadStartRoutine startRoutine,
                               XPR_ThreadEndRoutine endRoutine,
                               unsigned int flags, unsigned int stackSize,
                               void* opaque)
{
    XPR_Thread* t = (XPR_Thread*)calloc(sizeof(*t), 1);
    if (t) {
        t->handle = 0;
        t->startRoutine = startRoutine;
        t->endRoutine = endRoutine;
        t->flags = flags;
        t->opaque = opaque;
        t->userData = 0;
        if (pthread_create(&t->handle, 0, XPR_ThreadRoutineWrapper, t) < 0) {
            free((void*)t);
            t = 0;
        }
    }
    return t;
}

int XPR_ThreadDestroy(XPR_Thread* thread)
{
    if (thread && thread->handle != 0)
        XPR_ThreadJoin(thread);
    if (thread)
        free((void*)thread);
    return 0;
}

int XPR_ThreadJoin(XPR_Thread* thread)
{
    if (thread && thread->handle != 0)
        pthread_join(thread->handle, 0);
    return 0;
}

int64_t XPR_ThreadGetCurrentId(void)
{
    return (int64_t)pthread_self();
}

int64_t XPR_ThreadGetId(XPR_Thread* thread)
{
    if (thread)
        return (int64_t)thread->handle;
    return XPR_ThreadGetCurrentId();
}

void XPR_ThreadSleep(int64_t us)
{
    usleep(us);
}

void XPR_ThreadSleepEx(XPR_Thread* thread, int64_t us)
{
    usleep(us);
}

void XPR_ThreadYield(void)
{
    pthread_yield();
}
