/*
 * File: xpr_fq.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 弹性数据队列
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2016-12-28 07:58:50 Wednesday, 28 December
 * Last Modified : 2019-07-03 05:15:14 Wednesday, 3 July
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
#ifndef XPR_FQ_H
#define XPR_FQ_H

#include <stddef.h> // for size_t;
#include <stdint.h> // for int64_t;
#include <stdio.h> // for FILE*;
#include <xpr/xpr_common.h> // for XPR_API;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_FQ_TYPE_DEFINED
#define XPR_FQ_TYPE_DEFINED
///
/// FQ 对象数据结构前置声明
///
struct XPR_FQ;
///
/// FQ 对象数据类型声明
///
typedef struct XPR_FQ XPR_FQ;
#endif // XPR_FQ_TYPE_DEFINED

#ifndef XPR_FQ_ENTRY_TYPE_DEFINED
#define XPR_FQ_ENTRY_TYPE_DEFINED
struct XPR_FQ_ENTRY {
    uint8_t*    data;    ///< 数据地址
    size_t      length;  ///< 数据长度
    size_t
    flags;   ///< 数据标志（用于标记数据类型或用途，可以在取出时使用）
};

typedef struct XPR_FQ_ENTRY XPR_FQ_ENTRY;
#endif // XPR_FQ_ENTRY_TYPE_DEFINED

/// @brief 创建弹性数据队列实例。
/// @param [in] maxQueues           最大队列长度。
/// @param [in] maxBufferSize       占用内存最大字节数。
/// @retval NULL  创建失败
/// @retval Other 弹性数据队列实例地址。
XPR_API XPR_FQ* XPR_FQ_Create(size_t maxQueues, size_t maxBufferSize);

/// @brief 销毁弹性数据队列实例。
/// @param [in] fq                 弹性数据队列实例地址。
/// @retval XPR_ERR_GEN_NULL_PTR 用户传入空指针
/// @retval XPR_ERR_OK           清理成功
XPR_API int XPR_FQ_Destroy(XPR_FQ* fq);

/// @brief 清除队列数据
/// @param [in] fq                 弹性数据队列实例地址。
/// @retval XPR_ERR_GEN_NULL_PTR 用户传入空指针
/// @retval XPR_ERR_OK           清理成功
XPR_API int XPR_FQ_Clear(XPR_FQ* fq);

/// @brief 从弹性数据队列实例中弹出数据。
/// @param [in] fq                 弹性数据队列实例地址。
/// @retval NULL  队列为空
/// @retval Other 返回数据项信息
/// @note 当返回值不再使用时，必须调用 XPR_FQ_ReleaseEntry() 释放关联的资源。
XPR_API XPR_FQ_ENTRY* XPR_FQ_PopFront(XPR_FQ* fq);

/// @brief 释放从弹性数据队列实例中弹出数据所关联的资源
/// @param [in] fq                  弹性数据队列实例地址。
/// @param [in] entry               从弹性数据队列实例中弹出数据，即 XPR_FQ_PopFront() 的返回值。
/// @retval XPR_ERR_OK      释放成功。
/// @retval XPR_ERR_ERROR   释放失败。
XPR_API int XPR_FQ_ReleaseEntry(XPR_FQ* fq, XPR_FQ_ENTRY* entry);

/// @brief 将包含在 XPR_FQ_ENTRY 的数据压入队列。
/// @param [in] fq                  弹性数据队列实例地址。
/// @param [in] entry               手工填写的 XPR_FQ_ENTRY 或者 XPR_FQ_PopFront() 的返回值。
/// @retval XPR_ERR_OK          压入成功。
/// @retval XPR_ERR_GEN_NOBUF   没有足够的缓冲区。
XPR_API int XPR_FQ_PushBack(XPR_FQ* fq, const XPR_FQ_ENTRY* entry);

/// @brief 将原始数据压入队列。
/// @param [in] fq                  弹性数据队列实例地址。
/// @param [in] data                要压入的数据地址。
/// @param [in] length              要压入的数据字节数。
/// @param [in] flags               要压入的数据标志，用于标记数据类型或用途，可以在取出时使用。
/// @retval XPR_ERR_OK          压入成功。
/// @retval XPR_ERR_GEN_NOBUF   没有足够的缓冲区。
XPR_API int XPR_FQ_PushBackRaw(XPR_FQ* fq, const void* data, size_t length,
                               size_t flags);

#ifdef __cplusplus
}
#endif

#endif // XPR_FQ_H
