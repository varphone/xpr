#include <stdio.h>
#include <stdlib.h>
#if defined(WIN32) || defined(_WIN32)
#include <crtdbg.h> 
#endif
#include <process.h>
#include <Windows.h>
#include "sys.h"
#include "thread.h"


typedef NTSTATUS (NTAPI *NtWaitForSingleObjectProc)(HANDLE Handle, BOOLEAN Alertable, PLARGE_INTEGER Timeout);
typedef NTSTATUS (NTAPI *NtDelayExecutionProc)(BOOLEAN Alertable, PLARGE_INTEGER DelayInterval);

struct Thread {
    HANDLE handle;
    HANDLE sleepEvent;
    unsigned id;
    ThreadStartRoutine routine;
    void* opaque;
    void* userData;
    NtDelayExecutionProc NtDelayExecution;
    NtWaitForSingleObjectProc NtWaitForSingleObject;
};

static unsigned int __stdcall ThreadRoutineWrapper(void *opaque)
{
    Thread* thread = (Thread*)opaque;
    unsigned int result = 0;
    __try {
        result = (unsigned int)thread->routine(thread, thread->opaque);
    }
    __except(0) {
        OutputDebugStringA("thread->routine raise an error\n");
    }
    return result;
}

Thread* ThreadCreate(ThreadStartRoutine routine, unsigned int stackSize, void* opaque)
{
    Thread* t = (Thread*)calloc(sizeof(*t), 1);
    if (t) {
        t->routine = routine;
        t->opaque = opaque;
        t->handle = (HANDLE)_beginthreadex(0, stackSize, ThreadRoutineWrapper, t, 0, &t->id);
        if (t->handle == INVALID_HANDLE_VALUE) {
            free((void*)t);
            t = 0;
        }
        t->userData = 0;
        t->NtDelayExecution = (NtDelayExecutionProc)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtDelayExecution");
        //t->sleepEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
        //t->NtWaitForSingleObject = (NtWaitForSingleObjectProc)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtWaitForSingleObject");
    }
    return t;
}

int ThreadDestroy(Thread* thread)
{
    if (thread && thread->handle != INVALID_HANDLE_VALUE)
        ThreadJoin(thread);
    if (thread)
        free((void*)thread);
    return 0;
}

int ThreadJoin(Thread* thread)
{
    if (thread && thread->handle != INVALID_HANDLE_VALUE) {
        WaitForSingleObject(thread->handle, INFINITE);
        CloseHandle(thread->handle);
        thread->handle = INVALID_HANDLE_VALUE;
    }
    return 0;
}

int64_t ThreadGetCurrentId(void)
{
    return GetCurrentThreadId();
}

int64_t ThreadGetId(Thread* thread)
{
    if (thread)
        return thread->id;
    return ThreadGetCurrentId();
}

void ThreadSleep(int64_t usec)
{
#if 0
    static SOCKET s = 0;
    struct timeval tv;
    fd_set dummy;
    if (s == 0)
        s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    FD_ZERO(&dummy);
    FD_SET(s, &dummy);
    tv.tv_sec = usec/1000000L;
    tv.tv_usec = usec%1000000L;
    select(0, &dummy, 0, 0, &tv);
#else
    SleepEx(usec/1000, FALSE);
#endif
}

#if 0
void ThreadSleepEx(Thread* thread, int usec)
{
    LARGE_INTEGER tmo;
    if (thread && thread->NtWaitForSingleObject && thread->sleepEvent != INVALID_HANDLE_VALUE) {
        tmo.QuadPart = -usec*10;
        thread->NtWaitForSingleObject(thread->sleepEvent, FALSE, &tmo);
    }
}
#else
void ThreadSleepEx(Thread* thread, int64_t usec)
{
    LARGE_INTEGER tmo;
    if (thread && thread->NtDelayExecution) {
        tmo.QuadPart = -usec*10;
        thread->NtDelayExecution(FALSE, &tmo);
    }
    else
        Sleep(usec/1000);
}
#endif

void ThreadYield(void)
{
    SwitchToThread();
}

void ThreadSetUserData(Thread* thread, void* userData)
{
    if (thread)
        thread->userData = userData;
}

void* ThreadGetUserData(const Thread* thread)
{
    return thread ? thread->userData : 0;
}
