#include <Windows.h>
#include <intrin.h>
#include <xpr/xpr_atomic.h>
#include <xpr/xpr_sync.h>

void XPR_MutexInit(XPR_Mutex* mtx)
{
    *(HANDLE*)mtx->dummy = CreateMutex(NULL, FALSE, NULL);
}

void XPR_MutexFini(XPR_Mutex* mtx)
{
    (void)CloseHandle(*(HANDLE*)mtx->dummy);
}

void XPR_MutexLock(XPR_Mutex* mtx)
{
    (void)WaitForSingleObject(*(HANDLE*)mtx->dummy, INFINITE);
}

void XPR_MutexUnlock(XPR_Mutex* mtx)
{
    (void)ReleaseMutex(*(HANDLE*)mtx->dummy);
}

void XPR_RecursiveMutexInit(XPR_RecursiveMutex* mtx)
{
    (void)InitializeCriticalSectionAndSpinCount((CRITICAL_SECTION*)mtx->dummy, 1000);
}

void XPR_RecursiveMutexFini(XPR_RecursiveMutex* mtx)
{
    DeleteCriticalSection((CRITICAL_SECTION*)mtx->dummy);
}

void XPR_RecursiveMutexLock(XPR_RecursiveMutex* mtx)
{
    EnterCriticalSection((CRITICAL_SECTION*)mtx->dummy);
}

void XPR_RecursiveMutexUnlock(XPR_RecursiveMutex* mtx)
{
    LeaveCriticalSection((CRITICAL_SECTION*)mtx->dummy);
}

#define XPR_SPIN_LOCK_VALUE     1
#define XPR_SPIN_UNLOCK_VALUE   0

void XPR_SpinLockInit(XPR_SpinLock* s)
{
    if (s)
        XPR_AtomicAssign((XPR_Atomic*)s->dummy, XPR_SPIN_UNLOCK_VALUE);
}

void XPR_SpinLockFini(XPR_SpinLock* s)
{
    if (s)
        XPR_AtomicAssign((XPR_Atomic*)s->dummy, XPR_SPIN_UNLOCK_VALUE);
}

void XPR_SpinLockLock(XPR_SpinLock* s)
{
    int loops = 0;
    int missed = 0;
    if (s) {
        while (XPR_AtomicCAS((XPR_Atomic*)s->dummy,
                             XPR_SPIN_LOCK_VALUE,
                             XPR_SPIN_UNLOCK_VALUE) != XPR_SPIN_UNLOCK_VALUE)
        {
            while (loops++ < 500);
            loops = 0;
            if (missed++ > 5) {
                SwitchToThread();
                missed = 0;
            }
        }
    }
}

void XPR_SpinLockUnlock(XPR_SpinLock* s)
{
    if (s)
        XPR_AtomicAssign((XPR_Atomic*)s->dummy, XPR_SPIN_UNLOCK_VALUE);
}
