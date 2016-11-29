#ifndef XPR_MEM_H
#define XPR_MEM_H

#include <stdint.h>
#include <xpr/xpr_common.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @brief 分配内存资源
/// @param [in] size    数据大小，以字节为单位
/// @retval NULL    分配失败
/// @retval !NULL   已分配到的数据内存地址
/// @sa XPR_Free(), XPR_Freep()
XPR_API void* XPR_Alloc(size_t size);

/// @brief 释放内存资源
/// @param [in] ptr     已分配到的内存地址
/// @return 无返回值
/// @note 只能用于 XPR_Alloc() 分配的内存
/// @sa XPR_Alloc()
XPR_API void XPR_Free(void* ptr);

/// @brief 释放内存资源并将指针设为 NULL
/// @param [in] pptr    已分配到的内存地址的指针
/// @return 无返回值
/// @note 只能用于 XPR_Alloc() 分配的内存
/// @sa XPR_Alloc()
XPR_API void XPR_Freep(void** pptr);

/// @brief 释放列表型内存资源
/// @param [in] vpptr    已分配到的内存地址的指针列表
/// @return 无返回值
/// @note 只能用于 XPR_Alloc() 分配的内存
/// @note 列表数据格式为: vptr[0] = <p1>, vptr[1] = <p2>, vptr[n] = <pn> vptr[n+1] = null;
/// @sa XPR_Alloc()
XPR_API void XPR_Freev(void** vptr);

/// @brief 复制字符串
/// @param [in] str     要复制的字符串指针
/// @retval NULL    复制失败
/// @retval !NULL   复制后的字符串指针
/// @note 当返回值不再使用时请调用 XPR_Free() 释放其所占用的资源
XPR_API char* XPR_StrDup(const char* str);

/// @brief 复制字符串(宽字符版)
/// @param [in] str     要复制的字符串指针
/// @retval NULL    复制失败
/// @retval !NULL   复制后的字符串指针
/// @note 当返回值不再使用时请调用 XPR_Free() 释放其所占用的资源
XPR_API wchar_t* XPR_StrDupW(const wchar_t* str);

#ifdef __cplusplus
}
#endif

#endif // XPR_MEM_H

