/*
 * File: xpr_mem.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 内存管理接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2014-11-21 12:50:43 Friday, 21 November
 * Last Modified : 2019-07-03 04:55:46 Wednesday, 3 July
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
#ifndef XPR_MEM_H
#define XPR_MEM_H

#include <stddef.h>
#include <stdint.h>
#include <wchar.h>
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

/// @brief 分配带引用计数的内存资源
/// @param [in] size    数据大小，以字节为单位
/// @retval NULL    分配失败
/// @retval !NULL   已分配到的数据内存地址
/// @sa XPR_FreeRc()
XPR_API void* XPR_AllocRc(size_t size);

/// @brief 克隆带引用计数的内存资源
/// @param [in] ptr     已分配到的内存地址
/// @retval NULL    分配失败
/// @retval !NULL   已分配到的数据内存地址
/// @note 只能用于 XPR_AllocRc() 分配的内存
/// @sa XPR_AllocRc, XPR_FreeRc()
XPR_API void* XPR_CloneRc(void* ptr);

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

/// @brief 释放带引用计数的内存资源
/// @param [in] ptr     已分配到的内存地址
/// @return 无返回值
/// @note 只能用于 XPR_AllocRc() 分配的内存
/// @sa XPR_AllocRc()
XPR_API void XPR_FreeRc(void* ptr);

/// Mapping physical address into virtual address
/// @param [in] phyAddr     Physical address to mapping
/// @param [in] size        How much bytes to mapping
/// @return Mapped pointer or NULL on Error
XPR_API void* XPR_MemMap(uintptr_t phyAddr, size_t size);

/// Mapping file to memory with read and write access
/// @param [in] fileName    Filename to mapping
/// @param [in] size        File size in bytes
/// @return Mapped pointer or NULL on Error
XPR_API void* XPR_MemMapFile(const char* fileName, size_t size);

/// Mapping file to memory with read only access
/// @param [in] fileName    Filename to mapping
/// @param [in] size        File size in bytes
/// @return Mapped pointer or NULL on Error
XPR_API void* XPR_MemMapFileRO(const char* fileName, size_t size);

/// Release mapped virtual address
/// @param [in] virtAddr    Mapped virtual address
/// @param [in] size        Mapped bytes (for File Mapping Only)
/// @return XPR_ERR_OK or others on Error
/// @sa #XPR_MemMap, #XPR_MemMapFile, #XPR_MemMapFileRO
XPR_API int XPR_MemUnmap(void* virtAddr, size_t size);

/// @brief 获取带引用计数的内存资源的元数据
/// @param [in] ptr     已分配到的内存地址
/// @param [in] slot    元数据槽位：0~3
/// @return 成功返回槽位内当前值，失败返回 0
/// @note 只能用于 XPR_AllocRc() 分配的内存
/// @sa XPR_AllocRc()
XPR_API long XPR_RcGetMeta(void* ptr, int slot);

/// @brief 设置带引用计数的内存资源的元数据
/// @param [in] ptr     已分配到的内存地址
/// @param [in] slot    元数据槽位：0~3
/// @return 成功返回 XPR_TRUE, 失败返回 XPR_FALSE
/// @note 只能用于 XPR_AllocRc() 分配的内存
/// @sa XPR_AllocRc()
XPR_API int XPR_RcSetMeta(void* ptr, int slot, long val);

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
