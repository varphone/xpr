/*
 * File: xpr_thread.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 多线程操作接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2014-11-21 12:50:43 Friday, 21 November
 * Last Modified : 2019-07-03 04:35:00 Wednesday, 3 July
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
#ifndef XPR_THREAD_H
#define XPR_THREAD_H

#include <stdint.h>
#include <xpr/xpr_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XPR_THREAD_FLAG_AUTO_DESTROY    0x0001

#ifndef XPR_THREAD_TYPE_DEFINED
#define XPR_THREAD_TYPE_DEFINED
// 前置声明
struct XPR_Thread;
///
/// 线程对象类型定义
///
typedef struct XPR_Thread XPR_Thread;
#endif // XPR_THREAD_TYPE_DEFINED

/// @brief 线程入口函数
/// @param [in] thread      线程对象
/// @param [in] opaque      用户关联数据
/// @return 线程结束时返回的数据
typedef void* (*XPR_ThreadStartRoutine)(void* opaque, XPR_Thread* thread);

/// @brief 线程结束函数
/// @param [in] opaque      用户关联数据
/// @param [in] thread      线程对象
/// @return 无
typedef void (*XPR_ThreadEndRoutine)(void* opaque, XPR_Thread* thread);

/// @brief 创建一个新线程
/// @param [in] routine     线程入口函数
/// @param [in] stackSize   线程栈大小, 0 为系统默认值
/// @param [in] opaque      用户关联数据
/// @return 线程对象
XPR_API XPR_Thread* XPR_ThreadCreate(XPR_ThreadStartRoutine routine,
                                     unsigned int stackSize, void* opaque);

///
/// 创建一个新线程
///
/// @param [in] startRoutine    线程入口函数
/// @param [in] endRoutine      线程入口函数
/// @param [in] flags           线程创建标志
/// @param [in] stackSize       线程栈大小, 0 为系统默认值
/// @param [in] opaque          用户关联数据
/// @return 线程对象
XPR_API XPR_Thread* XPR_ThreadCreateEx(XPR_ThreadStartRoutine startRoutine,
                                       XPR_ThreadEndRoutine endRoutine,
                                       unsigned int flags,
                                       unsigned int stackSize, void* opaque);

/// @brief 销毁一个线程
/// @param [in] thread      线程对象
/// @retval XPR_ERR_SUCESS  销毁成功
/// @retval XPR_ERR_ERROR   销毁失败
/// @sa [#XPR_ErrorCode]
XPR_API int XPR_ThreadDestroy(XPR_Thread* thread);

/// @brief 等待线程退出
/// @param [in] thread      线程对象
/// @retval XPR_ERR_SUCESS  线程成功退出
/// @retval XPR_ERR_ERROR   等待线程退出时发生异常
XPR_API int XPR_ThreadJoin(XPR_Thread* thread);

/// @brief 获取当前线程的标识
/// @return 线程的标识
XPR_API int64_t XPR_ThreadGetCurrentId(void);

/// @brief 获取线程标识
/// @return 线程的标识
XPR_API int64_t XPR_ThreadGetId(XPR_Thread* thread);

/// @brief 使当前线程进入睡眠状态
/// @param [in] us          指定睡眠的时长，单位为 1/1000000 秒
/// @return 无
XPR_API void XPR_ThreadSleep(int64_t us);

/// @brief 使线程进入睡眠状态
/// @param [in] thread      线程对象
/// @param [in] us          指定睡眠的时长，单位为 1/1000000 秒
/// @return 无
XPR_API void XPR_ThreadSleepEx(XPR_Thread* thread, int64_t us);

/// @brief 使当前线程交出 CPU 资源
/// @return 无
XPR_API void XPR_ThreadYield(void);

/// @brief Set user data
XPR_API void XPR_ThreadSetUserData(XPR_Thread* thread, void* userData);

/// @brief Get user data
XPR_API void* XPR_ThreadGetUserData(const XPR_Thread* thread);

#ifdef __cplusplus
}
#endif

#endif // XPR_THREAD_H
