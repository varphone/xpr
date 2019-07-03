/*
 * File: xpr_atomic.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 原子操作接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2014-11-24 03:06:18 Monday, 24 November
 * Last Modified : 2019-07-03 05:31:33 Wednesday, 3 July
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
#ifndef XPR_ATOMIC_H
#define XPR_ATOMIC_H

#if defined(_MSC_VER)
#include <intrin.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
#if defined(_M_IX86)
typedef volatile long XPR_Atomic;
#define XPR_ATOMIC_MAX          0xffffffff
#define XPR_AtomicAssign(x, s)  _InterlockedExchange(x, s)
#define XPR_AtomicCAS(x, s, c)  _InterlockedCompareExchange(x, s, c)
#define XPR_AtomicRead(x)       (*(XPR_Atomic*)(x))
#define XPR_AtomicAdd(x, s)     _InterlockedExchangeAdd(x, s)
#define XPR_AtomicSub(x, s)     _InterlockedExchangeAdd(x, -(s))
#define XPR_AtomicDec(x)        _InterlockedDecrement(x)
#define XPR_AtomicInc(x)        _InterlockedIncrement(x)
#elif defined(_M_X64)
typedef volatile __int64 XPR_Atomic;
#define XPR_ATOMIC_MAX			0xffffffffffffffffLL
#define XPR_AtomicAssign(x, s)  _InterlockedExchange64(x, s)
#define XPR_AtomicCAS(x, s, c)  _InterlockedCompareExchange64(x, s, c)
#define XPR_AtomicRead(x)       (*(XPR_Atomic*)(x))
#define XPR_AtomicAdd(x, s)     _InterlockedExchangeAdd64(x, s)
#define XPR_AtomicSub(x, s)     _InterlockedExchangeAdd64(x, -(s))
#define XPR_AtomicDec(x)        _InterlockedDecrement64(x)
#define XPR_AtomicInc(x)        _InterlockedIncrement64(x)
#endif
#elif defined(__GNUC__)
typedef volatile long XPR_Atomic;
#define XPR_ATOMIC_MAX          0xffffffff
#define XPR_AtomicAssign(x, s)  __sync_val_compare_and_swap(x, s, s)
#define XPR_AtomicCAS(x, s, c)  __sync_val_compare_and_swap(x, c, s)
#define XPR_AtomicRead(x)       (*(XPR_Atomic*)(x))
#define XPR_AtomicAdd(x, s)     __sync_add_and_fetch(x, s)
#define XPR_AtomicSub(x, s)     __sync_sub_and_fetch(x, s)
#define XPR_AtomicDec(x)        __sync_sub_and_fetch(x, 1)
#define XPR_AtomicInc(x)        __sync_add_and_fetch(x, 1)
#endif

#ifdef __cplusplus
}
#endif

#endif // XPR_ATOMIC_H
