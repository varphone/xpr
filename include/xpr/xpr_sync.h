#ifndef XPR_SYNC_H
#define XPR_SYNC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_MUTEX_TYPE_DEFINED
#define XPR_MUTEX_TYPE_DEFINED
struct XPR_Mutex {
    uint8_t dummy[128];
};
typedef struct XPR_Mutex XPR_Mutex;
#endif // XPR_MUTEX_TYPE_DEFINED

#ifndef XPR_RECURSIVEMUTEX_TYPE_DEFINED
#define XPR_RECURSIVEMUTEX_TYPE_DEFINED
struct XPR_RecursiveMutex {
    uint8_t dummy[128];
};
typedef struct XPR_RecursiveMutex XPR_RecursiveMutex;
#endif // XPR_RECURSIVEMUTEX_TYPE_DEFINED

#ifndef XPR_SPINLOCK_TYPE_DEFINED
#define XPR_SPINLOCK_TYPE_DEFINED
struct XPR_SpinLock {
    uint8_t dummy[8];
};
typedef struct XPR_SpinLock XPR_SpinLock;
#endif // XPR_SPINLOCK_TYPE_DEFINED

void XPR_MutexInit(XPR_Mutex* mtx);
void XPR_MutexFini(XPR_Mutex* mtx);
void XPR_MutexLock(XPR_Mutex* mtx);
void XPR_MutexUnlock(XPR_Mutex* mtx);

void XPR_RecursiveMutexInit(XPR_RecursiveMutex* mtx);
void XPR_RecursiveMutexFini(XPR_RecursiveMutex* mtx);
void XPR_RecursiveMutexLock(XPR_RecursiveMutex* mtx);
void XPR_RecursiveMutexUnlock(XPR_RecursiveMutex* mtx);

void XPR_SpinLockInit(XPR_SpinLock* s);
void XPR_SpinLockFini(XPR_SpinLock* s);
void XPR_SpinLockLock(XPR_SpinLock* s);
void XPR_SpinLockUnlock(XPR_SpinLock* s);

#ifdef __cplusplus
}
#endif

#endif // XPR_SYNC_H

