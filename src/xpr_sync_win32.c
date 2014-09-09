#include <Windows.h>
#include <intrin.h>
#include "sync.h"

void MutexInit(Mutex* mtx)
{
    *(HANDLE*)mtx->dummy = CreateMutex(NULL, FALSE, NULL);
}

void MutexFini(Mutex* mtx)
{
    (void)CloseHandle(*(HANDLE*)mtx->dummy);
}

void MutexLock(Mutex* mtx)
{
    (void)WaitForSingleObject(*(HANDLE*)mtx->dummy, INFINITE);
}

void MutexUnlock(Mutex* mtx)
{
    (void)ReleaseMutex(*(HANDLE*)mtx->dummy);
}

void RecursiveMutexInit(RecursiveMutex* mtx)
{
    (void)InitializeCriticalSectionAndSpinCount((CRITICAL_SECTION*)mtx->dummy, 1000);
}

void RecursiveMutexFini(RecursiveMutex* mtx)
{
    DeleteCriticalSection((CRITICAL_SECTION*)mtx->dummy);
}

void RecursiveMutexLock(RecursiveMutex* mtx)
{
    EnterCriticalSection((CRITICAL_SECTION*)mtx->dummy);
}

void RecursiveMutexUnlock(RecursiveMutex* mtx)
{
    LeaveCriticalSection((CRITICAL_SECTION*)mtx->dummy);
}

void SpinLockInit(SpinLock* s)
{
    if (s)
        *((int*)s->dummy) = 0;
}

void SpinLockFini(SpinLock* s)
{
    if (s)
        *((int*)s->dummy) = 0;
}

void SpinLockLock(SpinLock* s)
{
    int loops = 0;
    int missed = 0;
    if (s) {
        while (_InterlockedCompareExchange((volatile long*)s->dummy, 1, 0) != 0) {
            while (loops++ < 500);
            loops = 0;
            if (missed++ > 5) {
                SwitchToThread();
                missed = 0;
            }
        }
    }
}

void SpinLockUnlock(SpinLock* s)
{
    if (s)
        _InterlockedExchange((volatile long*)s->dummy, 0);
}

