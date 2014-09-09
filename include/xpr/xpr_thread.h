#ifndef XPR_THREAD_H
#define XPR_THREAD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_THREAD_TYPE_DEFINED
#define XPR_THREAD_TYPE_DEFINED
struct XPR_Thread;
typedef struct XPR_Thread XPR_Thread; ///< XPR_Thread definition
#endif // XPR_THREAD_TYPE_DEFINED

/// @brief Thread start routine
typedef void* (*XPR_ThreadStartRoutine)(XPR_Thread* thread, void* opaque);

/// @brief Create a new thread
XPR_Thread* XPR_ThreadCreate(XPR_ThreadStartRoutine routine, unsigned int stackSize, void* opaque);

/// @brief Destroy a thread
int XPR_ThreadDestroy(XPR_Thread* thread);

/// @brief Join with a terminated thread
int XPR_ThreadJoin(XPR_Thread* thread);

/// @brief Get current thread id
int64_t XPR_ThreadGetCurrentId(void);

/// @brief Get thread id
int64_t XPR_ThreadGetId(XPR_Thread* thread);

void XPR_ThreadSleep(int64_t us);

void XPR_ThreadSleepEx(XPR_Thread* thread, int64_t us);

void XPR_ThreadYield(void);

/// @brief Set user data
void XPR_ThreadSetUserData(XPR_Thread* thread, void* userData);

/// @brief Get user data
void* XPR_ThreadGetUserData(const XPR_Thread* thread);

#ifdef __cplusplus
}
#endif

#endif // XPR_THREAD_H

