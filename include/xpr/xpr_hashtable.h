/*
 * File: xpr_hashtable.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 哈希表
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2019-07-12 09:02:31 Friday, 12 July
 * Last Modified : 2019-07-12 09:02:37 Friday, 12 July
 * Modified By   : Varphone Wong <varphone@qq.com>
 * ---------------------------------------------------------------------------
 * Copyright (C) 2012-2019 CETC55, Technology Development CO.,LTD.
 * Copyright (C) 2012-2019 Varphone Wong, Varphone.com.
 * All rights reserved.
 * ---------------------------------------------------------------------------
 * HISTORY:
 * 2019-07-12   varphone    创始版本建立
 */
#ifndef XPR_HASHTABLE_H
#define XPR_HASHTABLE_H

#include <stddef.h>
#include <stdint.h>
#include "xpr_common.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_HASHTABLE_TYPE_DEFINED
#define XPR_HASHTABLE_TYPE_DEFINED
struct XPR_HashTable;
typedef struct XPR_HashTable XPR_HashTable;
#endif // XPR_HASHTABLE_TYPE_DEFINED

/// 复制函数定义
typedef void* (*XPR_HashTableCopyFn)(void*);

/// 释放函数定义
typedef void (*XPR_HashTableDropFn)(void*);

/// 哈希函数定义
typedef uint32_t (*XPR_HashTableHashFn)(void*,uint32_t);

/// 验证函数定义
typedef int (*XPR_HashTableValidateFn)(void*);

#ifndef XPR_HASHTABLEALGO_TYPE_DEFINED
#define XPR_HASHTABLEALGO_TYPE_DEFINED
/// 哈希表算法
struct XPR_HashTableAlgo
{
    XPR_HashTableCopyFn keyCopy;         ///< 键的复制函数
    XPR_HashTableDropFn keyDrop;         ///< 键的释放函数
    XPR_HashTableHashFn keyHash;         ///< 键的哈希函数
    XPR_HashTableValidateFn keyValidate; ///< 键的验证函数
    XPR_HashTableCopyFn valueCopy;       ///< 值的复制函数
    XPR_HashTableDropFn valueDrop;       ///< 值的释放函数
};
typedef struct XPR_HashTableAlgo XPR_HashTableAlgo;
#endif // XPR_HASHTABLEALGO_TYPE_DEFINED

/// 创建一个哈希表
/// @param [in] capacity        指定表的容量
/// @param [in] algo            指定表的算法，NULL 为内置的 K/V 皆字串算法
/// @return 返回创建好的哈希表指针，失败时返回 NULL
XPR_API XPR_HashTable* XPR_HashTableCreate(size_t capacity,
                                           XPR_HashTableAlgo* algo);

/// 销毁一个已创建的哈希表
/// @param [in] self        哈希表指针
/// @return 无
XPR_API void XPR_HashTableDestroy(XPR_HashTable* self);

/// 清除哈希表中所有的项
/// @param [in] self        哈希表指针
/// @return 无
XPR_API void XPR_HashTableClear(XPR_HashTable* self);

/// 擦除指定键所对应的项
/// @param [in] self        哈希表指针
/// @param [in] key         要查询的键
/// @retval XPR_ERR_OK  擦除成功
/// @retval Others      擦除失败
XPR_API int XPR_HashTableErase(XPR_HashTable* self, void* key);

/// 返回指定键所对应的值
/// @param [in] self        哈希表指针
/// @param [in] key         要查询的键
/// @return 返回相应值，键不存在时返回 NULL
XPR_API void* XPR_HashTableGet(XPR_HashTable* self, void* key);

/// 返回首个迭代器
/// @param [in] self        哈希表指针
/// @return 返回相应指针，空表时返回 NULL
XPR_API void* XPR_HashTableIterFirst(XPR_HashTable* self);

/// 返回下个迭代器
/// @param [in] self        哈希表指针
/// @param [in] iter        当前迭代器指针
/// @return 返回相应指针，空表时返回 NULL
XPR_API void* XPR_HashTableIterNext(XPR_HashTable* self, void* iter);

/// 返回迭代器对应的键
/// @param [in] iter        迭代器指针
/// @return 返回相应值，错误时返回 NULL
XPR_API void* XPR_HashTableIterKey(void* iter);

/// 返回迭代器对应的值
/// @param [in] iter        迭代器指针
/// @return 返回相应值，错误时返回 NULL
XPR_API void* XPR_HashTableIterValue(void* iter);

/// 设置指定键所对应的值
/// @param [in] self        哈希表指针
/// @param [in] key         要查询的键
/// @param [in] value       要设置的值
/// @retval XPR_ERR_OK  设置成功
/// @retval Others      设置失败
XPR_API int XPR_HashTableSet(XPR_HashTable* self, void* key, void* value);

/// 返回已设置的项数量
/// @param [in] self        哈希表指针
/// @return 有效项数量
XPR_API int XPR_HashTableSize(XPR_HashTable* self);

#ifdef __cplusplus
}
#endif

#endif // XPR_HASHTABLE_H
