#include <stdio.h>
#include <stdlib.h>
#if defined(WIN32) || defined(_WIN32)
#include <crtdbg.h>
#endif
#include <Windows.h>
#include <process.h>
#include <xpr/xpr_sys.h>
#include <xpr/xpr_thread.h>

typedef NTSTATUS(NTAPI* NtWaitForSingleObjectProc)(HANDLE Handle,
                                                   BOOLEAN Alertable,
                                                   PLARGE_INTEGER Timeout);
typedef NTSTATUS(NTAPI* NtDelayExecutionProc)(BOOLEAN Alertable,
                                              PLARGE_INTEGER DelayInterval);

struct XPR_Thread
{
    HANDLE handle;
    HANDLE sleepEvent;
    unsigned id;
    XPR_ThreadStartRoutine startRoutine;
    XPR_ThreadEndRoutine endRoutine;
    unsigned flags;
    void* opaque;
    void* userData;
    NtDelayExecutionProc NtDelayExecution;
    NtWaitForSingleObjectProc NtWaitForSingleObject;
};

static unsigned int __stdcall ThreadRoutineWrapper(void* opaque)
{
    XPR_Thread* thread = (XPR_Thread*)opaque;
    unsigned int result = 0;
    __try {
        result = (uintptr_t)thread->startRoutine(thread->opaque, thread);
    }
    __except (0) {
        OutputDebugStringA("thread->startRoutine() raise an error\n");
    }
    if (thread->endRoutine)
        thread->endRoutine(thread->opaque, thread);
    return result;
}

XPR_API XPR_Thread* XPR_ThreadCreate(XPR_ThreadStartRoutine routine,
                                     unsigned int stackSize, void* opaque)
{
    XPR_Thread* t = (XPR_Thread*)calloc(sizeof(*t), 1);
    if (t) {
        t->startRoutine = routine;
        t->endRoutine = 0;
        t->flags = 0;
        t->opaque = opaque;
        t->handle = (HANDLE)_beginthreadex(0, stackSize, ThreadRoutineWrapper,
                                           t, 0, &t->id);
        if (t->handle == INVALID_HANDLE_VALUE) {
            free((void*)t);
            t = 0;
        }
        t->userData = 0;
        t->NtDelayExecution = (NtDelayExecutionProc)GetProcAddress(
            GetModuleHandleA("ntdll.dll"), "NtDelayExecution");
        // t->sleepEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
        // t->NtWaitForSingleObject =
        // (NtWaitForSingleObjectProc)GetProcAddress(GetModuleHandleA("ntdll.dll"),
        // "NtWaitForSingleObject");
    }
    return t;
}

XPR_API XPR_Thread* XPR_ThreadCreateEx(XPR_ThreadStartRoutine startRoutine,
                                       XPR_ThreadEndRoutine endRoutine,
                                       unsigned int flags,
                                       unsigned int stackSize, void* opaque)
{
    XPR_Thread* t = (XPR_Thread*)calloc(sizeof(*t), 1);
    if (t) {
        t->startRoutine = startRoutine;
        t->endRoutine = endRoutine;
        t->flags = flags;
        t->opaque = opaque;
        t->handle = (HANDLE)_beginthreadex(0, stackSize, ThreadRoutineWrapper,
                                           t, 0, &t->id);
        if (t->handle == INVALID_HANDLE_VALUE) {
            free((void*)t);
            t = 0;
        }
        t->userData = 0;
        t->NtDelayExecution = (NtDelayExecutionProc)GetProcAddress(
            GetModuleHandleA("ntdll.dll"), "NtDelayExecution");
        // t->sleepEvent = CreateEventW(NULL, TRUE, FALSE, NULL);
        // t->NtWaitForSingleObject =
        // (NtWaitForSingleObjectProc)GetProcAddress(GetModuleHandleA("ntdll.dll"),
        // "NtWaitForSingleObject");
    }
    return t;
}

XPR_API int XPR_ThreadDestroy(XPR_Thread* thread)
{
    if (thread && thread->handle != INVALID_HANDLE_VALUE)
        XPR_ThreadJoin(thread);
    if (thread)
        free((void*)thread);
    return 0;
}

XPR_API int XPR_ThreadJoin(XPR_Thread* thread)
{
    if (thread && thread->handle != INVALID_HANDLE_VALUE) {
        WaitForSingleObject(thread->handle, INFINITE);
        CloseHandle(thread->handle);
        thread->handle = INVALID_HANDLE_VALUE;
    }
    return 0;
}

XPR_API int64_t XPR_ThreadGetCurrentId(void)
{
    return GetCurrentThreadId();
}

XPR_API int64_t XPR_ThreadGetId(XPR_Thread* thread)
{
    if (thread)
        return thread->id;
    return XPR_ThreadGetCurrentId();
}

XPR_API void XPR_ThreadSleep(int64_t usec)
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
    SleepEx((DWORD)(usec / 1000), FALSE);
#endif
}

#if 0
XPR_API void ThreadSleepEx(Thread* thread, int usec)
{
    LARGE_INTEGER tmo;
    if (thread && thread->NtWaitForSingleObject && thread->sleepEvent != INVALID_HANDLE_VALUE) {
        tmo.QuadPart = -usec*10;
        thread->NtWaitForSingleObject(thread->sleepEvent, FALSE, &tmo);
    }
}
#else
XPR_API void XPR_ThreadSleepEx(XPR_Thread* thread, int64_t usec)
{
    LARGE_INTEGER tmo;
    if (thread && thread->NtDelayExecution) {
        tmo.QuadPart = -usec * 10;
        thread->NtDelayExecution(FALSE, &tmo);
    }
    else
        Sleep((DWORD)(usec / 1000));
}
#endif

XPR_API void XPR_ThreadYield(void)
{
    SwitchToThread();
}

XPR_API void XPR_ThreadSetUserData(XPR_Thread* thread, void* userData)
{
    if (thread)
        thread->userData = userData;
}

XPR_API void* XPR_ThreadGetUserData(const XPR_Thread* thread)
{
    return thread ? thread->userData : 0;
}
