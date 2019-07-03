/*
 * File: xpr_sync.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 同步相关操作接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2014-11-21 12:50:43 Friday, 21 November
 * Last Modified : 2019-07-03 04:37:00 Wednesday, 3 July
 * Modified By   : Varphone Wong <varphone@qq.com>
 * ---------------------------------------------------------------------------
 * Copyright (C) 2012 - 2019 CETC55, Technology Development CO.,LTD.
 * Copyright (C) 2012 - 2019 Varphone Wong, Varphone.com.
 * All rights reserved.
 * ---------------------------------------------------------------------------
 * HISTORY:
 * 2019-07-03   varphone    更新版权信息
 * 2014-11-21   varphone    初始版本建立
 */
#ifndef XPR_SYNC_H
#define XPR_SYNC_H

#include <stdint.h>
#include <xpr/xpr_common.h>

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

// 互斥锁
XPR_API void XPR_MutexInit(XPR_Mutex* mtx);
XPR_API void XPR_MutexFini(XPR_Mutex* mtx);
XPR_API void XPR_MutexLock(XPR_Mutex* mtx);
XPR_API void XPR_MutexUnlock(XPR_Mutex* mtx);

// 可递归互斥锁
XPR_API void XPR_RecursiveMutexInit(XPR_RecursiveMutex* mtx);
XPR_API void XPR_RecursiveMutexFini(XPR_RecursiveMutex* mtx);
XPR_API void XPR_RecursiveMutexLock(XPR_RecursiveMutex* mtx);
XPR_API void XPR_RecursiveMutexUnlock(XPR_RecursiveMutex* mtx);

// 自旋锁
XPR_API void XPR_SpinLockInit(XPR_SpinLock* s);
XPR_API void XPR_SpinLockFini(XPR_SpinLock* s);
XPR_API void XPR_SpinLockLock(XPR_SpinLock* s);
XPR_API void XPR_SpinLockUnlock(XPR_SpinLock* s);

#ifdef __cplusplus
}
#endif

#endif // XPR_SYNC_H
