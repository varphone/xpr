#include <pthread.h>
#include <xpr/xpr_sync.h>

void XPR_MutexInit(XPR_Mutex* mtx)
{
    pthread_mutex_init((pthread_mutex_t*)mtx->dummy, NULL);
}

void XPR_MutexFini(XPR_Mutex* mtx)
{
    pthread_mutex_destroy((pthread_mutex_t*)mtx->dummy);
}

void XPR_MutexLock(XPR_Mutex* mtx)
{
    pthread_mutex_lock((pthread_mutex_t*)mtx->dummy);
}

void XPR_MutexUnlock(XPR_Mutex* mtx)
{
    pthread_mutex_unlock((pthread_mutex_t*)mtx->dummy);
}

void XPR_RecursiveMutexInit(XPR_RecursiveMutex* mtx)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init((pthread_mutex_t*)mtx->dummy, &attr);
    pthread_mutexattr_destroy(&attr);
}

void XPR_RecursiveMutexFini(XPR_RecursiveMutex* mtx)
{
    pthread_mutex_destroy((pthread_mutex_t*)mtx->dummy);
}

void XPR_RecursiveMutexLock(XPR_RecursiveMutex* mtx)
{
    pthread_mutex_lock((pthread_mutex_t*)mtx->dummy);
}

void XPR_RecursiveMutexUnlock(XPR_RecursiveMutex* mtx)
{
    pthread_mutex_unlock((pthread_mutex_t*)mtx->dummy);
}

void XPR_SpinLockInit(XPR_SpinLock* s)
{
    long* v = 0;
    if (s) {
        v = (long*)s->dummy;
        *v = 0;
    }
}

void XPR_SpinLockFini(XPR_SpinLock* s)
{
    long* v = 0;
    if (s) {
        v = (long*)s->dummy;
        *v = 0;
    }
}

void XPR_SpinLockLock(XPR_SpinLock* s)
{
    int loops = 0;
    if (s) {
        while (__sync_lock_test_and_set((long*)(s->dummy), 1) == 1) {
            if (loops++ > 1000) {
                pthread_yield();
                loops = 0;
            }
        }
    }
}

void XPR_SpinLockUnlock(XPR_SpinLock* s)
{
    if (s)
        __sync_lock_release((long*)(s->dummy));
}

