#ifndef XPR_THREAD_H
#define XPR_THREAD_H

#include <stdint.h>

/// @defgroup xprthread 线程
/// @brief 多线程操作接口
/// @{
///

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_THREAD_TYPE_DEFINED
#define XPR_THREAD_TYPE_DEFINED
struct XPR_Thread;
typedef struct XPR_Thread XPR_Thread; ///< XPR_Thread definition
#endif // XPR_THREAD_TYPE_DEFINED

/// @brief 线程入口函数
/// @param [in] thread      线程对象
/// @param [in] opaque      用户关联数据
/// @return 线程结束时返回的数据
typedef void* (*XPR_ThreadStartRoutine)(XPR_Thread* thread, void* opaque);

/// @brief 创建一个新线程
/// @param [in] routine     线程入口函数
/// @param [in] stackSize   线程栈大小, 0 为系统默认值
/// @param [in] opaque      用户关联数据
/// @return 线程对象
XPR_Thread* XPR_ThreadCreate(XPR_ThreadStartRoutine routine, unsigned int stackSize, void* opaque);

/// @brief 销毁一个线程
/// @param [in] thread      线程对象
/// @retval XPR_ERR_SUCESS  销毁成功
/// @retval XPR_ERR_ERROR   销毁失败
/// @sa [#XPR_ErrorCode]
int XPR_ThreadDestroy(XPR_Thread* thread);

/// @brief 等待线程退出
/// @param [in] thread      线程对象
/// @retval XPR_ERR_SUCESS  线程成功退出
/// @retval XPR_ERR_ERROR   等待线程退出时发生异常
int XPR_ThreadJoin(XPR_Thread* thread);

/// @brief 获取当前线程的标识
/// @return 线程的标识
int64_t XPR_ThreadGetCurrentId(void);

/// @brief 获取线程标识
/// @return 线程的标识
int64_t XPR_ThreadGetId(XPR_Thread* thread);

/// @brief 使当前线程进入睡眠状态
/// @param [in] us          指定睡眠的时长，单位为 1/1000000 秒
/// return 无
void XPR_ThreadSleep(int64_t us);

/// @brief 使线程进入睡眠状态
/// @param [in] thread      线程对象
/// @param [in] us          指定睡眠的时长，单位为 1/1000000 秒
/// return 无
void XPR_ThreadSleepEx(XPR_Thread* thread, int64_t us);

/// @brief 使当前线程交出 CPU 资源
/// return 无
void XPR_ThreadYield(void);

/// @brief Set user data
void XPR_ThreadSetUserData(XPR_Thread* thread, void* userData);

/// @brief Get user data
void* XPR_ThreadGetUserData(const XPR_Thread* thread);

#ifdef __cplusplus
}
#endif

/// @}
///

#endif // XPR_THREAD_H

