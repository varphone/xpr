/*
 * File: xpr_ups.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 采用 K/V 形式的通用参数设定操作接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2014-11-21 12:50:43 Friday, 21 November
 * Last Modified : 2019-07-03 04:11:00 Wednesday, 3 July
 * Modified By   : Varphone Wong <varphone@qq.com>
 * ---------------------------------------------------------------------------
 * Copyright (C) 2012 - 2019 CETC55, Technology Development CO.,LTD.
 * Copyright (C) 2012 - 2019 Varphone Wong, Varphone.com.
 * All rights reserved.
 * ---------------------------------------------------------------------------
 * HISTORY:
 * 2019-07-03   varphone    更新版权信息
 * 2012-12-20   varphone    初始版本建立
 */
#ifndef XPR_UPS_H
#define XPR_UPS_H

#include <stdint.h>
#include <xpr/xpr_common.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_json.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XPR_UPS_MAX_KEY_LEN 1024 ///< 键最大长度
#define XPR_UPS_VERSION XPR_MakeVersion(1, 0, 0)

/// Case XPR_UPS_Node* to XPR_UPS_Entry*
#define XPR_UPS_TO_ENTRY(node) (XPR_UPS_Entry*)(node)

/// Case XPR_UPS_Entry* to XPR_UPS_Node*
#define XPR_UPS_TO_NODE(entry) (XPR_UPS_Node*)(entry)

// Return only type bits from the `flags|type`
#define XPR_UPS_TO_TYPE(type) (XPR_UPS_EntryType)((type) & 0xff)

// Return true if type == XPR_UPS_ENTRY_TYPE_DIR
#define XPR_UPS_ENTRY_IS_DIR(entry)                                            \
    (XPR_UPS_TO_TYPE(entry->type) == XPR_UPS_ENTRY_TYPE_DIR)

// Return true if type == XPR_UPS_ENTRY_TYPE_INIT
#define XPR_UPS_ENTRY_IS_INIT(entry)                                           \
    (XPR_UPS_TO_TYPE((entry)->type) == XPR_UPS_ENTRY_TYPE_INIT)

/// Return the parent entry
#define XPR_UPS_ENTRY_PATERN(entry) ((XPR_UPS_Entry*)(entry)->node.parent)

#ifndef XPR_UPS_BLOB_TYPE_DEFINED
#define XPR_UPS_BLOB_TYPE_DEFINED
/// 二进制型设定项数据
struct XPR_UPS_Blob {
    void* data; ///< 数据指针
    int size;   ///< 数据大小
};
typedef struct XPR_UPS_Blob XPR_UPS_Blob;
#endif // XPR_UPS_BLOB_TYPE_DEFINED

#ifndef XPR_UPS_ENTRY_TYPE_DEFINED
#define XPR_UPS_ENTRY_TYPE_DEFINED
/// 设定项前置声明
struct XPR_UPS_Entry;
typedef struct XPR_UPS_Entry XPR_UPS_Entry;
#endif // XPR_UPS_ENTRY_TYPE_DEFINED

#ifndef XPR_UPS_ENTRYFLAGS_TYPE_DEFINED
#define XPR_UPS_ENTRYFLAGS_TYPE_DEFINED
/// 设定项额外标志
enum XPR_UPS_EntryFlags
{
    XPR_UPS_ENTRY_FLAT_INITLA = 1 << 9, ///< 延后调用 init()，与 INITAR 配合
    XPR_UPS_ENTRY_FLAG_INITAR = 1 << 10, ///< 注册后调用 init()，与 INITLA 配合
    XPR_UPS_ENTRY_FLAG_NOSTOR = 1 << 11, ///< 数据不需要存储
    XPR_UPS_ENTRY_FLAG_SHADOW = 1 << 12, ///< 数据缓存至阴影位置
};
typedef enum XPR_UPS_EntryFlags XPR_UPS_EntryFlags;
#endif // XPR_UPS_ENTRYFLAGS_TYPE_DEFINED

#ifndef XPR_UPS_ENTRYTYPE_TYPE_DEFINED
#define XPR_UPS_ENTRYTYPE_TYPE_DEFINED
/// 设定项类型
enum XPR_UPS_EntryType
{
    XPR_UPS_ENTRY_TYPE_UNKNOWN, ///< 未知
    XPR_UPS_ENTRY_TYPE_INIT,    ///< 初始化使用的特殊节点
    XPR_UPS_ENTRY_TYPE_DIR,     ///< 目录
    XPR_UPS_ENTRY_TYPE_BOOLEAN, ///< 布尔
    XPR_UPS_ENTRY_TYPE_BLOB,    ///< 二进制数据
    XPR_UPS_ENTRY_TYPE_I32,     ///< 32 位整形
    XPR_UPS_ENTRY_TYPE_I64,     ///< 32 位整形
    XPR_UPS_ENTRY_TYPE_F32,     ///< 32 位浮点数
    XPR_UPS_ENTRY_TYPE_F64,     ///< 64 位浮点数
    XPR_UPS_ENTRY_TYPE_STRING,  ///< 字符串
    XPR_UPS_ENTRY_TYPE_INT = XPR_UPS_ENTRY_TYPE_I32,
    XPR_UPS_ENTRY_TYPE_REAL = XPR_UPS_ENTRY_TYPE_F64,
};
typedef enum XPR_UPS_EntryType XPR_UPS_EntryType;
#endif // XPR_UPS_ENTRYTYPE_TYPE_DEFINED

#ifndef XPR_UPS_NODE_YPE_DEFINED
#define XPR_UPS_NODE_YPE_DEFINED
/// 设定项节点前置声明
struct XPR_UPS_Node;
typedef struct XPR_UPS_Node XPR_UPS_Node;
#endif // XPR_UPS_NODE_YPE_DEFINED

#ifndef XPR_UPS_VALUE_YPE_DEFINED
#define XPR_UPS_VALUE_YPE_DEFINED
/// 设定项数据值前置声明
union XPR_UPS_Value;
typedef union XPR_UPS_Value XPR_UPS_Value;
#endif // XPR_UPS_VALUE_YPE_DEFINED

/// @brief 设定项初始化函数
/// @param [in] ent     设定项指针
/// @return see [#XPR_ErrorCode]
typedef int (*XPR_UPS_Initlializer)(XPR_UPS_Entry* ent);

#define XPR_UPS_DEF_INITER(fn) static int fn(XPR_UPS_Entry* entry)

/// @brief 设定项释放函数
/// @param [in] ent     设定项指针
/// @return see [#XPR_ErrorCode]
typedef int (*XPR_UPS_Finalizer)(XPR_UPS_Entry* ent);

#define XPR_UPS_DEF_FINIER(fn) static int fn(XPR_UPS_Entry* entry)

/// @brief 设定项数据获取函数
/// @param [in] ent         设定项指针
/// @param [in] key         设定项的键名
/// @param [in,out] buffer  用于接受获取到的数据的缓冲区
/// @param [in,out] size    指示用于接受获取到的数据的缓冲区大小，
///                         返回实际获取到的字节数
/// @retval XPR_ERR_OK  获取成功
/// @retval Other       获取发生异常, 请查看 [#XPR_ErrorCode]
typedef int (*XPR_UPS_Getter)(XPR_UPS_Entry* ent, const char* key, void* buffer,
                              int* size);

#define XPR_UPS_DEF_GETTER(fn)                                                 \
    static int fn(XPR_UPS_Entry* entry, const char* key, void* buffer,         \
                  int* size)

/// @brief 设定项数据设定函数
/// @param [in] ent         设定项指针
/// @param [in] key         设定项的键名
/// @param [in,out] data    用于设定的数据
/// @param [in,out] size    用于设定的数据字节数
/// @retval XPR_ERR_OK  设定成功
/// @retval Other       设定发生异常, 请查看 [#XPR_ErrorCode]
typedef int (*XPR_UPS_Setter)(XPR_UPS_Entry* ent, const char* key,
                              const void* data, int size);

#define XPR_UPS_DEF_SETTER(fn)                                                 \
    static int fn(XPR_UPS_Entry* entry, const char* key, const void* data,     \
                  int size)

/// 设定项节点结构
struct XPR_UPS_Node
{
    struct XPR_UPS_Node* parent; ///< 父节点（上级）
    struct XPR_UPS_Node* prev;   ///< 前一个节点（同级）
    struct XPR_UPS_Node* next;   ///< 下一个节点（同级）
    struct XPR_UPS_Node* childs; ///< 首个子节点（下级）
};

/// 设定项数据值
union XPR_UPS_Value
{
    // 注意：字段定义需要从字节数大到小排列
    int64_t i64;     ///< 64 位整形数值
    double f64;      ///< 64 位浮点数值
    XPR_UPS_Blob bb; ///< 二进制数据
    char* str;       ///< 字符串值
    int32_t i32;     ///< 32 位整形数值
    float f32;       ///< 32 位浮点数值
    char bl;         ///< 布尔值
};

/// 设定项结构
/// @note `type` 低 8  位为 XPR_UPS_EntryType
///              高 24 位为 XPR_UPS_EntryFlags
struct XPR_UPS_Entry
{
    XPR_UPS_Node node;
    const char* name; ///< 设定项名称, 可包含多个, 最后一个必须以 NULL 作为结束
    const char* desc; ///< 设定项描述, 可包含多个, 最后一个必须以 NULL 作为结束
    const char* category;      ///< 设定项所属类别
    const char* root;          ///< 设定项所属目录
    int type;                  ///< 设定项所含数据的类型，额外标志位
    XPR_UPS_Initlializer init; ///< 设定项初始化函数
    XPR_UPS_Finalizer fini;    ///< 设定项释放函数
    XPR_UPS_Getter get;        ///< 设定项数据获取函数
    XPR_UPS_Setter set;        ///< 设定项数据设定函数
    XPR_UPS_Value curVal;      ///< 当前值
    XPR_UPS_Value defVal;      ///< 默认值
    XPR_UPS_Value shaVal;      ///< 幽灵值
};

// 定义一个设定项（名称、描述、类别、存根、类型、初始、终释、获取、设定）
#define XPR_UPS_ENTRY_PAR(name, desc, category, root, type, init, fini, get,   \
                          set)                                                 \
    {                                                                          \
        {0, 0, 0, 0}, name, desc, category, root, type, init, fini, get, set,  \
            {0}, {0}, {0},                                                     \
    }

// 定义一个设定项（名称、描述、类别、存根、类型、初始、终释、获取、设定、默认值）
#define XPR_UPS_ENTRY_PAR_DV(name, desc, category, root, type, init, fini,     \
                             get, set, defVal)                                 \
    {                                                                          \
        {0, 0, 0, 0}, name, desc, category, root, type, init, fini, get, set,  \
            {0}, {defVal}, {0},                                                \
    }

// 定义一个设定项（名称、类型）
#define XPR_UPS_ENTRY_PAR1(name, type)                                         \
    XPR_UPS_ENTRY_PAR(name, "", "ups/par", NULL, type, NULL, NULL, NULL, NULL)

// 定义一个设定项（名称、类型、默认值）
#define XPR_UPS_ENTRY_PAR1_DV(name, type, defVal)                              \
    XPR_UPS_ENTRY_PAR_DV(name, "", "ups/par", NULL, type, NULL, NULL, NULL,    \
                         NULL, defVal)

// 定义一个设定项（名称、类型、getter）
#define XPR_UPS_ENTRY_PAR1_G(name, type, getter)                               \
    XPR_UPS_ENTRY_PAR(name, "", "ups/par", NULL, type, NULL, NULL, getter, NULL)

// 定义一个设定项（名称、类型、getter、默认值）
#define XPR_UPS_ENTRY_PAR1_G_DV(name, type, getter, defVal)                    \
    XPR_UPS_ENTRY_PAR_DV(name, "", "ups/par", NULL, type, NULL, NULL, getter,  \
                         NULL, defVal)

// 定义一个设定项（名称、类型、getter、setter）
#define XPR_UPS_ENTRY_PAR1_G_S(name, type, getter, stter)                      \
    XPR_UPS_ENTRY_PAR(name, "", "ups/par", NULL, type, NULL, NULL, getter,     \
                      setter)

// 定义一个设定项（名称、类型、getter、setter、默认值）
#define XPR_UPS_ENTRY_PAR1_G_S_DV(name, type, getter, stter, defVal)           \
    XPR_UPS_ENTRY_PAR_DV(name, "", "ups/par", NULL, type, NULL, NULL, getter,  \
                         setter, defVal)

// 定义一个设定项（名称、类型、setter）
#define XPR_UPS_ENTRY_PAR1_S(name, type, setter)                               \
    XPR_UPS_ENTRY_PAR(name, "", "ups/par", NULL, type, NULL, NULL, NULL, setter)

// 定义一个设定项（名称、类型、setter、默认值）
#define XPR_UPS_ENTRY_PAR1_S_DV(name, type, setter, defVal)                    \
    XPR_UPS_ENTRY_PAR_DV(name, "", "ups/par", NULL, type, NULL, NULL, NULL,    \
                         setter, defVal)

// 定义一个设定项（名称、描述、类型）
#define XPR_UPS_ENTRY_PAR2(name, desc, type)                                   \
    XPR_UPS_ENTRY_PAR(name, desc, "ups/par", NULL, type, NULL, NULL, NULL, NULL)

// 定义一个设定项（名称、描述、类型、默认值）
#define XPR_UPS_ENTRY_PAR2_DV(name, desc, type, defVal)                        \
    XPR_UPS_ENTRY_PAR_DV(name, desc, "ups/par", NULL, type, NULL, NULL, NULL,  \
                         NULL, defVal)

// 定义一个设定项（名称、描述、存根、类型、getter）
#define XPR_UPS_ENTRY_PAR2_G(name, desc, type, getter)                         \
    XPR_UPS_ENTRY_PAR(name, desc, "ups/par", NULL, type, NULL, NULL, getter,   \
                      NULL)

// 定义一个设定项（名称、描述、存根、类型、getter、默认值）
#define XPR_UPS_ENTRY_PAR2_G_DV(name, desc, type, getter, defVal)              \
    XPR_UPS_ENTRY_PAR_DV(name, desc, "ups/par", NULL, type, NULL, NULL,        \
                         getter, NULL, defVal)

// 定义一个设定项（名称、描述、存根、类型、getter、setter）
#define XPR_UPS_ENTRY_PAR2_G_S(name, desc, type, getter, setter, defVal)       \
    XPR_UPS_ENTRY_PAR(name, desc, "ups/par", NULL, type, NULL, NULL, getter,   \
                      setter)

// 定义一个设定项（名称、描述、存根、类型、getter、setter、默认值）
#define XPR_UPS_ENTRY_PAR2_G_S_DV(name, desc, type, getter, setter, defVal)    \
    XPR_UPS_ENTRY_PAR_DV(name, desc, "ups/par", NULL, type, NULL, NULL,        \
                         getter, setter, defVal)

// 定义一个设定项（名称、描述、存根、类型、setter）
#define XPR_UPS_ENTRY_PAR2_S(name, desc, type, setter)                         \
    XPR_UPS_ENTRY_PAR(name, desc, "ups/par", NULL, type, NULL, NULL, NULL,     \
                      setter)

// 定义一个设定项（名称、描述、存根、类型、setter、默认值）
#define XPR_UPS_ENTRY_PAR2_S_DV(name, desc, type, setter, defVal)              \
    XPR_UPS_ENTRY_PAR_DV(name, desc, "ups/par", NULL, type, NULL, NULL, NULL,  \
                         setter, defVal)

// 定义一个设定项（名称、描述、类别、类型）
#define XPR_UPS_ENTRY_PAR3(name, desc, category, type)                         \
    XPR_UPS_ENTRY_PAR(name, desc, category, NULL, type, NULL, NULL, NULL, NULL)

// 定义一个设定项（名称、描述、类别、类型、默认值）
#define XPR_UPS_ENTRY_PAR3_DV(name, desc, category, type, defVal)              \
    XPR_UPS_ENTRY_PAR_DV(name, desc, category, NULL, type, NULL, NULL, NULL,   \
                         NULL, defVal)

// 定义一个设定项（名称、描述、类别、类型、getter）
#define XPR_UPS_ENTRY_PAR3_G(name, desc, category, type, getter)               \
    XPR_UPS_ENTRY_PAR(name, desc, category, NULL, type, NULL, NULL, getter,    \
                      NULL)

// 定义一个设定项（名称、描述、类别、类型、getter、默认值）
#define XPR_UPS_ENTRY_PAR3_G_DV(name, desc, category, type, getter, defVal)    \
    XPR_UPS_ENTRY_PAR_DV(name, desc, category, NULL, type, NULL, NULL, getter, \
                         NULL, defVal)

// 定义一个设定项（名称、描述、类别、类型、getter、setter）
#define XPR_UPS_ENTRY_PAR3_G_S(name, desc, category, type, getter, setter)     \
    XPR_UPS_ENTRY_PAR(name, desc, category, NULL, type, NULL, NULL, getter,    \
                      setter)

// 定义一个设定项（名称、描述、类别、类型、getter、setter、默认值）
#define XPR_UPS_ENTRY_PAR3_G_S_DV(name, desc, category, type, getter, setter,  \
                                  defVal)                                      \
    XPR_UPS_ENTRY_PAR_DV(name, desc, category, NULL, type, NULL, NULL, getter, \
                         setter, defVal)

// 定义一个设定项（名称、描述、类别、类型、setter、默认值）
#define XPR_UPS_ENTRY_PAR3_S(name, desc, category, type, setter, defVal)       \
    XPR_UPS_ENTRY_PAR(name, desc, category, NULL, type, NULL, NULL, NULL,      \
                      setter)

// 定义一个设定项（名称、描述、类别、类型、setter、默认值）
#define XPR_UPS_ENTRY_PAR3_S_DV(name, desc, category, type, getter, setter,    \
                                defVal)                                        \
    XPR_UPS_ENTRY_PAR_DV(name, desc, category, NULL, type, NULL, NULL, NULL,   \
                         setter, defVal)

// 定义一个设定项（名称、描述、类别、存根、类型）
#define XPR_UPS_ENTRY_PAR4(name, desc, category, root, type)                   \
    XPR_UPS_ENTRY_PAR(name, desc, category, root, type, NULL, NULL, NULL, NULL)

// 定义一个设定项（名称、描述、类别、存根、类型、默认值）
#define XPR_UPS_ENTRY_PAR4_DV(name, desc, category, root, type, defVal)        \
    XPR_UPS_ENTRY_PAR_DV(name, desc, category, root, type, NULL, NULL, NULL,   \
                         NULL, defVal)

// 定义一个设定项（名称、描述、类别、存根、类型、getter）
#define XPR_UPS_ENTRY_PAR4_G(name, desc, category, root, type, getter)         \
    XPR_UPS_ENTRY_PAR(name, desc, category, root, type, NULL, NULL, getter,    \
                      NULL)

// 定义一个设定项（名称、描述、类别、存根、类型、getter、默认值）
#define XPR_UPS_ENTRY_PAR4_G_DV(name, desc, category, root, type, getter,      \
                                defVal)                                        \
    XPR_UPS_ENTRY_PAR_DV(name, desc, category, root, type, NULL, NULL, getter, \
                         NULL, defVal)

// 定义一个设定项（名称、描述、类别、存根、类型、getter、setter）
#define XPR_UPS_ENTRY_PAR4_G_S(name, desc, category, root, type, getter,       \
                               setter)                                         \
    XPR_UPS_ENTRY_PAR(name, desc, category, root, type, NULL, NULL, getter,    \
                      setter)

// 定义一个设定项（名称、描述、类别、存根、类型、getter、setter、默认值）
#define XPR_UPS_ENTRY_PAR4_G_S_DV(name, desc, category, root, type, getter,    \
                                  setter, defVal)                              \
    XPR_UPS_ENTRY_PAR_DV(name, desc, category, root, type, NULL, NULL, getter, \
                         setter, defVal)

// 定义一个设定项（名称、描述、类别、存根、类型、setter、默认值）
#define XPR_UPS_ENTRY_PAR4_S(name, desc, category, root, type, setter, defVal) \
    XPR_UPS_ENTRY_PAR(name, desc, category, root, type, NULL, NULL, NULL,      \
                      setter)

// 定义一个设定项（名称、描述、类别、存根、类型、setter、默认值）
#define XPR_UPS_ENTRY_PAR4_S_DV(name, desc, category, root, type, getter,      \
                                setter, defVal)                                \
    XPR_UPS_ENTRY_PAR_DV(name, desc, category, root, type, NULL, NULL, NULL,   \
                         setter, defVal)

// 定义一个初始化设定项（名称、初始）
#define XPR_UPS_ENTRY_INIT1_I(name, init)                                      \
    XPR_UPS_ENTRY_PAR(name, "", "ups/init", NULL, XPR_UPS_ENTRY_TYPE_INIT,     \
                      init, NULL, NULL, NULL)

// 定义一个初始化设定项（名称、描述、初始）
#define XPR_UPS_ENTRY_INIT2_I(name, desc, init)                                \
    XPR_UPS_ENTRY_PAR(name, desc, "ups/init", NULL, XPR_UPS_ENTRY_TYPE_INIT,   \
                      init, NULL, NULL, NULL)

// 定义一个初始化设定项（名称、描述、类别、初始）
#define XPR_UPS_ENTRY_INIT3_I(name, desc, category, init)                      \
    XPR_UPS_ENTRY_PAR(name, desc, category, NULL, XPR_UPS_ENTRY_TYPE_INIT,     \
                      init, NULL, NULL, NULL)

// 定义一个初始化设定项（名称、描述、类别、存根、初始）
#define XPR_UPS_ENTRY_INIT4_I(name, desc, category, root, init)                \
    XPR_UPS_ENTRY_PAR(name, desc, category, root, XPR_UPS_ENTRY_TYPE_INIT,     \
                      init, NULL, NULL, NULL)

// 定义一个设定项目录（名称、描述）
#define XPR_UPS_ENTRY_DIR1(name)                                               \
    XPR_UPS_ENTRY_PAR(name, "", "ups/dir", NULL, XPR_UPS_ENTRY_TYPE_DIR, NULL, \
                      NULL, NULL, NULL)

// 定义一个设定项目录（名称、描述、获取、设定）
#define XPR_UPS_ENTRY_DIR1_G_S(name, getter, setter)                           \
    XPR_UPS_ENTRY_PAR(name, "", "ups/dir", NULL, XPR_UPS_ENTRY_TYPE_DIR, NULL, \
                      NULL, getter, setter)

// 定义一个设定项目录（名称、描述）
#define XPR_UPS_ENTRY_DIR2(name, desc)                                         \
    XPR_UPS_ENTRY_PAR(name, desc, "ups/dir", NULL, XPR_UPS_ENTRY_TYPE_DIR,     \
                      NULL, NULL, NULL, NULL)

// 定义一个设定项目录（名称、描述、获取、设定）
#define XPR_UPS_ENTRY_DIR2_G_S(name, desc, getter, setter)                     \
    XPR_UPS_ENTRY_PAR(name, desc, "ups/dir", NULL, XPR_UPS_ENTRY_TYPE_DIR,     \
                      NULL, NULL, getter, setter)

// 定义一个设定项目录（名称、描述、类别）
#define XPR_UPS_ENTRY_DIR3(name, desc, category)                               \
    XPR_UPS_ENTRY_PAR(name, desc, category, NULL, XPR_UPS_ENTRY_TYPE_DIR,      \
                      NULL, NULL, NULL, NULL)

// 定义一个设定项目录（名称、描述、类别、存根）
#define XPR_UPS_ENTRY_DIR4(name, desc, category, root)                         \
    XPR_UPS_ENTRY_PAR(name, desc, category, root, XPR_UPS_ENTRY_TYPE_DIR,      \
                      NULL, NULL, NULL, NULL)

// 定义一个设定项目录（名称、描述、类别、存根、初始、终释）
#define XPR_UPS_ENTRY_DIR4_I_F(name, desc, category, root, init, fini)         \
    XPR_UPS_ENTRY_PAR(name, desc, category, root, XPR_UPS_ENTRY_TYPE_DIR,      \
                      init, fini, NULL, NULL)

// 定义一个设定项目录（名称、描述、类别、存根、初始、终释、获取、设定）
#define XPR_UPS_ENTRY_DIR4_I_F_G_S(name, desc, category, root, init, fini,     \
                                   getter, setter)                             \
    XPR_UPS_ENTRY_PAR(name, desc, category, root, XPR_UPS_ENTRY_TYPE_DIR,      \
                      init, fini, getter, setter)

// 定义一个设定项目录（名称、描述、类别、存根、获取、设定）
#define XPR_UPS_ENTRY_DIR4_G_S(name, desc, category, root, getter, setter)     \
    XPR_UPS_ENTRY_PAR4_G_S(name, desc, category, root, XPR_UPS_ENTRY_TYPE_DIR, \
                           getter, setter)

// 定义一个布尔型设定项（名称、描述）
#define XPR_UPS_ENTRY_PAR_BOOL(name, desc)                                     \
    XPR_UPS_ENTRY_PAR2(name, desc, XPR_UPS_ENTRY_TYPE_BOOL)

// 定义一个布尔型设定项（名称、描述、默认值）
#define XPR_UPS_ENTRY_PAR_BOOL_DV(name, desc, defVal)                          \
    XPR_UPS_ENTRY_PAR2_DV(name, desc, XPR_UPS_ENTRY_TYPE_BOOL, defVal)

// 定义一个布尔型设定项（名称、描述、getter）
#define XPR_UPS_ENTRY_PAR_BOOL_G(name, desc, getter)                           \
    XPR_UPS_ENTRY_PAR2_G(name, desc, XPR_UPS_ENTRY_TYPE_BOOL, getter)

// 定义一个布尔型设定项（名称、描述、getter、默认值）
#define XPR_UPS_ENTRY_PAR_BOOL_G_DV(name, desc, getter, defVal)                \
    XPR_UPS_ENTRY_PAR2_G_DV(name, desc, XPR_UPS_ENTRY_TYPE_BOOL, getter, defVal)

// 定义一个布尔型设定项（名称、描述、getter、setter）
#define XPR_UPS_ENTRY_PAR_BOOL_G_S(name, desc, getter, setter)                 \
    XPR_UPS_ENTRY_PAR2_G_S(name, desc, XPR_UPS_ENTRY_TYPE_BOOL, getter, setter)

// 定义一个布尔型设定项（名称、描述、getter、setter、默认值）
#define XPR_UPS_ENTRY_PAR_BOOL_G_S_DV(name, desc, getter, setter, defVal)      \
    XPR_UPS_ENTRY_PAR2_G_S_DV(name, desc, XPR_UPS_ENTRY_TYPE_BOOL, getter,     \
                              setter, defVal)

// 定义一个布尔型设定项（名称、描述、setter）
#define XPR_UPS_ENTRY_PAR_BOOL_S(name, desc, setter)                           \
    XPR_UPS_ENTRY_PAR2_S(name, desc, XPR_UPS_ENTRY_TYPE_BOOL, setter)

// 定义一个布尔型设定项（名称、描述、setter、默认值）
#define XPR_UPS_ENTRY_PAR_BOOL_S_DV(name, desc, setter, defVal)                \
    XPR_UPS_ENTRY_PAR2_S_DV(name, desc, XPR_UPS_ENTRY_TYPE_BOOL, setter, defVal)

// 定义一个 32 位整型设定项（名称、描述）
#define XPR_UPS_ENTRY_PAR_I32(name, desc)                                      \
    XPR_UPS_ENTRY_PAR2(name, desc, XPR_UPS_ENTRY_TYPE_I32)

// 定义一个 32 位整型设定项（名称、描述、默认值）
#define XPR_UPS_ENTRY_PAR_I32_DV(name, desc, defVal)                           \
    XPR_UPS_ENTRY_PAR2_DV(name, desc, XPR_UPS_ENTRY_TYPE_I32, defVal)

// 定义一个 32 位整型设定项（名称、描述、getter）
#define XPR_UPS_ENTRY_PAR_I32_G(name, desc, getter)                            \
    XPR_UPS_ENTRY_PAR2_G(name, desc, XPR_UPS_ENTRY_TYPE_I32, getter)

// 定义一个 32 位整型设定项（名称、描述、getter、默认值）
#define XPR_UPS_ENTRY_PAR_I32_G_DV(name, desc, getter, defVal)                 \
    XPR_UPS_ENTRY_PAR2_G_DV(name, desc, XPR_UPS_ENTRY_TYPE_I32, getter, defVal)

// 定义一个 32 位整型设定项（名称、描述、getter、setter）
#define XPR_UPS_ENTRY_PAR_I32_G_S(name, desc, getter, setter)                  \
    XPR_UPS_ENTRY_PAR2_G_S(name, desc, XPR_UPS_ENTRY_TYPE_I32, getter, setter)

// 定义一个 32 位整型设定项（名称、描述、getter、setter、默认值）
#define XPR_UPS_ENTRY_PAR_I32_G_S_DV(name, desc, getter, setter, defVal)       \
    XPR_UPS_ENTRY_PAR2_G_S_DV(name, desc, XPR_UPS_ENTRY_TYPE_I32, getter,      \
                              setter, defVal)

// 定义一个 32 位整型设定项（名称、描述、setter）
#define XPR_UPS_ENTRY_PAR_I32_S(name, desc, setter)                            \
    XPR_UPS_ENTRY_PAR2_S(name, desc, XPR_UPS_ENTRY_TYPE_I32, setter)

// 定义一个 32 位整型设定项（名称、描述、setter、默认值）
#define XPR_UPS_ENTRY_PAR_I32_S_DV(name, desc, setter, defVal)                 \
    XPR_UPS_ENTRY_PAR2_S_DV(name, desc, XPR_UPS_ENTRY_TYPE_I32, setter, defVal)

// 定义一个 64 位整型设定项（名称、描述）
#define XPR_UPS_ENTRY_PAR_I64(name, desc)                                      \
    XPR_UPS_ENTRY_PAR2(name, desc, XPR_UPS_ENTRY_TYPE_I64)

// 定义一个 64 位整型设定项（名称、描述、默认值）
#define XPR_UPS_ENTRY_PAR_I64_DV(name, desc, defVal)                           \
    XPR_UPS_ENTRY_PAR2_DV(name, desc, XPR_UPS_ENTRY_TYPE_I64, defVal)

// 定义一个 64 位整型设定项（名称、描述、getter）
#define XPR_UPS_ENTRY_PAR_I64_G(name, desc, getter)                            \
    XPR_UPS_ENTRY_PAR2_G(name, desc, XPR_UPS_ENTRY_TYPE_I64, getter)

// 定义一个 64 位整型设定项（名称、描述、getter、默认值）
#define XPR_UPS_ENTRY_PAR_I64_G_DV(name, desc, getter, defVal)                 \
    XPR_UPS_ENTRY_PAR2_G_DV(name, desc, XPR_UPS_ENTRY_TYPE_I64, getter, defVal)

// 定义一个 64 位整型设定项（名称、描述、getter、setter）
#define XPR_UPS_ENTRY_PAR_I64_G_S(name, desc, getter, setter)                  \
    XPR_UPS_ENTRY_PAR2_G_S(name, desc, XPR_UPS_ENTRY_TYPE_I64, getter, setter)

// 定义一个 64 位整型设定项（名称、描述、getter、setter、默认值）
#define XPR_UPS_ENTRY_PAR_I64_G_S_DV(name, desc, getter, setter, defVal)       \
    XPR_UPS_ENTRY_PAR2_G_S_DV(name, desc, XPR_UPS_ENTRY_TYPE_I64, getter,      \
                              setter, defVal)

// 定义一个 64 位整型设定项（名称、描述、setter）
#define XPR_UPS_ENTRY_PAR_I64_S(name, desc, setter)                            \
    XPR_UPS_ENTRY_PAR2_S(name, desc, XPR_UPS_ENTRY_TYPE_I64, setter)

// 定义一个 64 位整型设定项（名称、描述、setter、默认值）
#define XPR_UPS_ENTRY_PAR_I64_S_DV(name, desc, setter, defVal)                 \
    XPR_UPS_ENTRY_PAR2_S_DV(name, desc, XPR_UPS_ENTRY_TYPE_I64, setter, defVal)

// 定义一个 32 位浮点型设定项（名称、描述）
#define XPR_UPS_ENTRY_PAR_F32(name, desc)                                      \
    XPR_UPS_ENTRY_PAR2(name, desc, XPR_UPS_ENTRY_TYPE_F32)

// 定义一个 32 位浮点型设定项（名称、描述、默认值）
#define XPR_UPS_ENTRY_PAR_F32_DV(name, desc, defVal)                           \
    XPR_UPS_ENTRY_PAR2_DV(name, desc, XPR_UPS_ENTRY_TYPE_F32, defVal)

// 定义一个 32 位浮点型设定项（名称、描述、getter）
#define XPR_UPS_ENTRY_PAR_F32_G(name, desc, getter)                            \
    XPR_UPS_ENTRY_PAR2_G(name, desc, XPR_UPS_ENTRY_TYPE_F32, setter)

// 定义一个 32 位浮点型设定项（名称、描述、getter、默认值）
#define XPR_UPS_ENTRY_PAR_F32_G_DV(name, desc, getter, defVal)                 \
    XPR_UPS_ENTRY_PAR2_G_DV(name, desc, XPR_UPS_ENTRY_TYPE_F32, setter, defVal)

// 定义一个 32 位浮点型设定项（名称、描述、getter、setter）
#define XPR_UPS_ENTRY_PAR_F32_G_S(name, desc, getter, setter)                  \
    XPR_UPS_ENTRY_PAR2_G_S(name, desc, XPR_UPS_ENTRY_TYPE_F32, getter, setter)

// 定义一个 32 位浮点型设定项（名称、描述、getter、setter、默认值）
#define XPR_UPS_ENTRY_PAR_F32_G_S_DV(name, desc, getter, setter, defVal)       \
    XPR_UPS_ENTRY_PAR2_G_S_DV(name, desc, XPR_UPS_ENTRY_TYPE_F32, getter,      \
                              setter, defVal)

// 定义一个 32 位浮点型设定项（名称、描述、setter）
#define XPR_UPS_ENTRY_PAR_F32_S(name, desc, setter)                            \
    XPR_UPS_ENTRY_PAR2_S(name, desc, XPR_UPS_ENTRY_TYPE_F32, setter)

// 定义一个 32 位浮点型设定项（名称、描述、setter、默认值）
#define XPR_UPS_ENTRY_PAR_F32_S_DV(name, desc, setter, defVal)                 \
    XPR_UPS_ENTRY_PAR2_S_DV(name, desc, XPR_UPS_ENTRY_TYPE_F32, setter, defVal)

// 定义一个 64 位浮点型设定项（名称、描述）
#define XPR_UPS_ENTRY_PAR_F64(name, desc)                                      \
    XPR_UPS_ENTRY_PAR2(name, desc, XPR_UPS_ENTRY_TYPE_F64)

// 定义一个 64 位浮点型设定项（名称、描述、默认值）
#define XPR_UPS_ENTRY_PAR_F64_DV(name, desc, defVal)                           \
    XPR_UPS_ENTRY_PAR2_DV(name, desc, XPR_UPS_ENTRY_TYPE_F64, defVal)

// 定义一个 64 位浮点型设定项（名称、描述、getter）
#define XPR_UPS_ENTRY_PAR_F64_G(name, desc, getter)                            \
    XPR_UPS_ENTRY_PAR2_G(name, desc, XPR_UPS_ENTRY_TYPE_F64, getter)

// 定义一个 64 位浮点型设定项（名称、描述、getter、默认值）
#define XPR_UPS_ENTRY_PAR_F64_G_DV(name, desc, getter, defVal)                 \
    XPR_UPS_ENTRY_PAR2_G_DV(name, desc, XPR_UPS_ENTRY_TYPE_F64, getter, defVal)

// 定义一个 64 位浮点型设定项（名称、描述、getter、setter）
#define XPR_UPS_ENTRY_PAR_F64_G_S(name, desc, getter, setter)                  \
    XPR_UPS_ENTRY_PAR2_G_S(name, desc, XPR_UPS_ENTRY_TYPE_F64, getter, setter)

// 定义一个 64 位浮点型设定项（名称、描述、getter、setter、默认值）
#define XPR_UPS_ENTRY_PAR_F64_G_S_DV(name, desc, getter, setter, defVal)       \
    XPR_UPS_ENTRY_PAR2_G_S_DV(name, desc, XPR_UPS_ENTRY_TYPE_F64, getter,      \
                              setter, defVal)

// 定义一个 64 位浮点型设定项（名称、描述、setter）
#define XPR_UPS_ENTRY_PAR_F64_S(name, desc, setter)                            \
    XPR_UPS_ENTRY_PAR2_S(name, desc, XPR_UPS_ENTRY_TYPE_F64, setter)

// 定义一个字串设定项（名称、描述）
#define XPR_UPS_ENTRY_PAR_STR(name, desc)                                      \
    XPR_UPS_ENTRY_PAR2(name, desc, XPR_UPS_ENTRY_TYPE_STRING)

// 定义一个字串设定项（名称、描述、默认值）
#define XPR_UPS_ENTRY_PAR_STR_DV(name, desc, defVal)                           \
    XPR_UPS_ENTRY_PAR2_DV(name, desc, XPR_UPS_ENTRY_TYPE_STRING, defVal)

// 定义一个字串设定项（名称、描述、getter）
#define XPR_UPS_ENTRY_PAR_STR_G(name, desc, getter)                            \
    XPR_UPS_ENTRY_PAR2_G(name, desc, XPR_UPS_ENTRY_TYPE_STRING, getter)

// 定义一个字串设定项（名称、描述、getter、默认值）
#define XPR_UPS_ENTRY_PAR_STR_G_DV(name, desc, getter, defVal)                 \
    XPR_UPS_ENTRY_PAR2_G_DV(name, desc, XPR_UPS_ENTRY_TYPE_STRING, getter,     \
                            defVal)

// 定义一个字串设定项（名称、描述、getter、setter）
#define XPR_UPS_ENTRY_PAR_STR_G_S(name, desc, getter, setter)                  \
    XPR_UPS_ENTRY_PAR2_G_S(name, desc, XPR_UPS_ENTRY_TYPE_STRING, getter,      \
                           setter)

// 定义一个字串设定项（名称、描述、getter、setter、默认值）
#define XPR_UPS_ENTRY_PAR_STR_G_S_DV(name, desc, getter, defVal)               \
    XPR_UPS_ENTRY_PAR2_G_S_DV(name, desc, XPR_UPS_ENTRY_TYPE_STRING, getter,   \
                              setter, defVal)

// 定义一个字串设定项（名称、描述、setter）
#define XPR_UPS_ENTRY_PAR_STR_S(name, desc, setter)                            \
    XPR_UPS_ENTRY_PAR2_S(name, desc, XPR_UPS_ENTRY_TYPE_STRING, setter)

// 定义一个字串设定项（名称、描述、setter、默认值）
#define XPR_UPS_ENTRY_PAR_STR_S_DV(name, desc, setter, defVal)                 \
    XPR_UPS_ENTRY_PAR2_S_DV(name, desc, XPR_UPS_ENTRY_TYPE_STRING, setter,     \
                            defVal)

// 定义一个设定项结束项
#define XPR_UPS_ENTRY_END()                                                    \
    {                                                                          \
        {0, 0, 0, 0}, NULL, NULL, NULL, NULL, XPR_UPS_ENTRY_TYPE_UNKNOWN,      \
            NULL, NULL, NULL, NULL, {0}, {0}, {0},                             \
    }

/// @brief 打开设定集
/// @param [in] storage     设定项数据存储路径
/// @retval <=0 失败
/// @retval >0  已打开的设定集实例
XPR_API int XPR_UPS_Init(const char* storage);

/// @brief 关闭已打开的设定集
/// @retval -1  失败
/// @retval 0   成功
XPR_API int XPR_UPS_Fini(void);

/// 查找指定设定项节点
/// @param [in] key         设定项名称
/// @param [in] parent      设定项父节点，可以为 NULL
/// @retval !NULL 设定项节点指针
/// @retval NULL  设定项没有注册
XPR_API XPR_UPS_Entry* XPR_UPS_FindEntry(const char* key,
                                         XPR_UPS_Entry* parent);

/// @brief 注册设定项节点
/// @param [in] ents        需要添加的节点数组
/// @param [in] count       需要添加的节点数
/// @retval 见错误码表
XPR_API int XPR_UPS_Register(XPR_UPS_Entry ents[], int count);

/// @brief 在指定目录注册设定项节点
/// @param [in] ents        需要添加的节点数组
/// @param [in] count       需要添加的节点数
/// @param [in] root        要注册节点的目录
/// @retval 见错误码表
XPR_API int XPR_UPS_RegisterAt(XPR_UPS_Entry ents[], int count,
                               const char* root);

/// @brief 在指定父节点上注册设定项节点
/// @param [in] entry       需要添加的节点
/// @param [in] parent      需要添加的父节点，可以为 NULL
/// @retval 见错误码表
XPR_API int XPR_UPS_RegisterSingle(XPR_UPS_Entry* entry, XPR_UPS_Entry* parent);

/// @brief 卸载设定项节点
/// @param [in] ents        需要卸载的节点数组
/// @param [in] ecount      需要卸载的节点数
XPR_API int XPR_UPS_UnRegister(XPR_UPS_Entry ents[], int count);

XPR_API int XPR_UPS_UnRegisterAll(void);

/// @brief 设置字串型设定项值
/// @param [in] key         设定项名称
/// @param [in] value       设定项值
/// @retval 见错误码表
XPR_API int XPR_UPS_SetString(const char* key, const char* value, int size);
XPR_API int XPR_UPS_SetStringVK(const char* value, int size, const char* vkey,
                                ...);

/// @brief 获取字串型设定项值
/// @param [in] key         设定项名称
/// @retval NULL    设定项不存在或设定项值为空
/// @retval 见错误码表
XPR_API int XPR_UPS_GetString(const char* key, char* value, int* size);
XPR_API int XPR_UPS_GetStringVK(char* value, int* size, const char* vkey, ...);

/// 直接获取设定项缓存的字串值
/// @param [in] key         设定项名称
/// @return 返回最后设定的值或 NULL
/// @note 本接口会跳过 getter 回调
XPR_API const char* XPR_UPS_PeekString(const char* key);
XPR_API const char* XPR_UPS_PeekStringVK(const char* vkey, ...);

/// @brief 设置整数型设定项值
/// @param [in] key         设定项名称
/// @param [in] value       设定项值
/// @retval 见错误码表
XPR_API int XPR_UPS_SetInteger(const char* key, int value);
XPR_API int XPR_UPS_SetIntegerVK(int value, const char* vkey, ...);

/// @brief 获取整数型设定项值
/// @param [in] key         设定项名称
/// @retval 见错误码表
XPR_API int XPR_UPS_GetInteger(const char* key, int* value);
XPR_API int XPR_UPS_GetIntegerVK(int* value, const char* vkey, ...);

/// 直接获取设定项缓存的整数值
/// @param [in] key         设定项名称
/// @return 返回最后设定的值或 0
/// @note 本接口会跳过 getter 回调
XPR_API int XPR_UPS_PeekInteger(const char* key);
XPR_API int XPR_UPS_PeekIntegerVK(const char* vkey, ...);

/// @brief 设置 64 位整数型设定项值
/// @param [in] token       已打开的设定实例
/// @param [in] key         设定项名称
/// @param [in] value       设定项值
/// @retval 见错误码表
XPR_API int XPR_UPS_SetInt64(const char* key, int64_t value);
XPR_API int XPR_UPS_SetInt64VK(int64_t value, const char* vkey, ...);

/// @brief 获取 64 位整数型设定项值
/// @param [in] token       已打开的设定实例
/// @param [in] key         设定项名称
/// @retval 见错误码表
XPR_API int XPR_UPS_GetInt64(const char* key, int64_t* value);
XPR_API int XPR_UPS_GetInt64VK(int64_t* value, const char* vkey, ...);

/// 直接获取设定项缓存的 64 位整数值
/// @param [in] key         设定项名称
/// @return 返回最后设定的值或 0
/// @note 本接口会跳过 getter 回调
XPR_API int64_t XPR_UPS_PeekInt64(const char* key);
XPR_API int64_t XPR_UPS_PeekInt64VK(const char* vkey, ...);

/// @brief 设置浮点数型设定项值
/// @param [in] token       已打开的设定实例
/// @param [in] key         设定项名称
/// @param [in] value       设定项值
/// @retval 见错误码表
XPR_API int XPR_UPS_SetFloat(const char* key, float value);
XPR_API int XPR_UPS_SetFloatVK(float value, const char* vkey, ...);

/// @brief 获取浮点数型设定项值
/// @param [in] token       已打开的设定实例
/// @param [in] key         设定项名称
/// @retval 见错误码表
XPR_API int XPR_UPS_GetFloat(const char* key, float* value);
XPR_API int XPR_UPS_GetFloatVK(float* value, const char* vkey, ...);

/// 直接获取设定项缓存的 32 位浮点值
/// @param [in] key         设定项名称
/// @return 返回最后设定的值或 0.0
/// @note 本接口会跳过 getter 回调
XPR_API float XPR_UPS_PeekFloat(const char* key);
XPR_API float XPR_UPS_PeekFloatVK(const char* vkey, ...);

/// @brief 设置双精度浮点数型设定项值
/// @param [in] token       已打开的设定实例
/// @param [in] key         设定项名称
/// @param [in] value       设定项值
/// @retval 见错误码表
XPR_API int XPR_UPS_SetDouble(const char* key, double value);
XPR_API int XPR_UPS_SetDoubleVK(double value, const char* vkey, ...);

/// @brief 获取双精度浮点数型设定项值
/// @param [in] token       已打开的设定实例
/// @param [in] key         设定项名称
/// @retval 见错误码表
XPR_API int XPR_UPS_GetDouble(const char* key, double* value);
XPR_API int XPR_UPS_GetDoubleVK(double* value, const char* vkey, ...);

/// 直接获取设定项缓存的 64 位浮点值
/// @param [in] key         设定项名称
/// @return 返回最后设定的值或 0.0
/// @note 本接口会跳过 getter 回调
XPR_API double XPR_UPS_PeekDouble(const char* key);
XPR_API double XPR_UPS_PeekDoubleVK(const char* vkey, ...);

/// @brief 设置布尔型设定项值
/// @param [in] token       已打开的设定实例
/// @param [in] key         设定项名称
/// @param [in] value       设定项值
/// @retval 见错误码表
XPR_API int XPR_UPS_SetBoolean(const char* key, int value);
XPR_API int XPR_UPS_SetBooleanVK(int value, const char* vkey, ...);

/// @brief 获取布尔型设定项值
/// @param [in] token       已打开的设定实例
/// @param [in] key         设定项名称
/// @retval 见错误码表
XPR_API int XPR_UPS_GetBoolean(const char* key, int* value);
XPR_API int XPR_UPS_GetBooleanVK(int* value, const char* vkey, ...);

/// 直接获取设定项缓存的布尔值
/// @param [in] key         设定项名称
/// @return 返回最后设定的值或 0/false
/// @note 本接口会跳过 getter 回调
XPR_API int XPR_UPS_PeekBoolean(const char* key);
XPR_API int XPR_UPS_PeekBooleanVK(const char* vkey, ...);

/// @brief 设置二进制型设定项值
/// @param [in] token       已打开的设定实例
/// @param [in] key         设定项名称
/// @param [in] value       设定项值
/// @retval 见错误码表
XPR_API int XPR_UPS_SetBlob(const char* key, XPR_UPS_Blob blob);
XPR_API int XPR_UPS_SetBlobVK(XPR_UPS_Blob blob, const char* vkey, ...);

/// @brief 获取二进制型设定项值
/// @param [in] token       已打开的设定实例
/// @param [in] key         设定项名称
/// @retval 见错误码表;
XPR_API int XPR_UPS_GetBlob(const char* key, XPR_UPS_Blob* value);
XPR_API int XPR_UPS_GetBlobVK(XPR_UPS_Blob* value, const char* vkey, ...);

/// 直接获取设定项缓存的二进制值
/// @param [in] key         设定项名称
/// @return 返回最后设定的值或 {NULL,0}
/// @note 本接口会跳过 getter 回调
XPR_API XPR_UPS_Blob XPR_UPS_PeekBlob(const char* key);
XPR_API XPR_UPS_Blob XPR_UPS_PeekBlobVK(const char* vkey, ...);

/// @brief 从存储文件或设备中读取对应数据
/// @param [in] ent         要读取的设定项指针
/// @param [in,out] buffer  用于接受读取到的数据的缓冲区
/// @param [in,out] size    指示用于接受读取到的数据的缓冲区大小，
///                         返回实际读取到的字节数
/// @retval XPR_ERR_OK  读取成功
/// @retval Other       读取发生异常, 请查看 [#XPR_ErrorCode]
XPR_API int XPR_UPS_ReadData(XPR_UPS_Entry* ent, void* buffer, int* size);

/// @brief 读取设定项的数据值
/// @param [in] entry       要读取的设定项指针
/// @param [in,out] buffer  用于接受读取到的数据的缓冲区
/// @param [in,out] size    指示用于接受读取到的数据的缓冲区大小，
///                         返回实际读取到的字节数
/// @retval XPR_ERR_OK  读取成功
/// @retval Other       读取发生异常, 请查看 [#XPR_ErrorCode]
XPR_API int XPR_UPS_ReadValue(XPR_UPS_Entry* entry, void* buffer, int* size);

/// @brief 将数据写入储文件或设备中
/// @param [in] ent         要写入的设定项指针
/// @param [in,out] data    用于写入的数据
/// @param [in,out] size    用于写入的数据字节数
/// @retval XPR_ERR_OK  写入成功
/// @retval Other       写入发生异常, 请查看 [#XPR_ErrorCode]
XPR_API int XPR_UPS_WriteData(XPR_UPS_Entry* ent, const void* data, int size);

/// @brief 更新设定项的数据值
/// @param [in] ent         要写入的设定项指针
/// @param [in,out] data    用于写入的数据
/// @param [in,out] size    用于写入的数据字节数
/// @retval XPR_ERR_OK  写入成功
/// @retval Other       写入发生异常, 请查看 [#XPR_ErrorCode]
XPR_API int XPR_UPS_WriteValue(XPR_UPS_Entry* entry, const void* data,
                               int size);

/// 将值放入目标缓冲区（Put In Buffer）
XPR_API int XPR_UPS_PibString(void* dst, int* dstSize, const char* val,
                              int valSize);
XPR_API int XPR_UPS_PibInteger(void* dst, int* dstSize, int val);
XPR_API int XPR_UPS_PibInt64(void* dst, int* dstSize, int64_t val);
XPR_API int XPR_UPS_PibFloat(void* dst, int* dstSize, float val);
XPR_API int XPR_UPS_PibDouble(void* dst, int* dstSize, double val);
XPR_API int XPR_UPS_PibBoolean(void* dst, int* dstSize, int val);
XPR_API int XPR_UPS_PibBlob(void* dst, int* dstSize, XPR_UPS_Blob val);

/// @brief 删除指定设定项
/// @param [in] key         设定项名称
/// @retval 见错误码表
XPR_API int XPR_UPS_Delete(const char* key);
XPR_API int XPR_UPS_DeleteVK(const char* vkey, ...);

/// @brief 检测设定项是否存在
/// @param [in] key         设定项名称
/// @retval 见错误码表
XPR_API int XPR_UPS_Exists(const char* key);
XPR_API int XPR_UPS_ExistsVK(const char* vkey, ...);

/// @brief 获取设定中的首个设定项名称
/// @retval 见错误码表
XPR_API const char* XPR_UPS_FirstKey(void);

/// @brief 获取设定中的下一个键名
/// @param [in] key         当前键名
/// @retval 见错误码表
XPR_API const char* XPR_UPS_NextKey(const char* key);

/// @brief 开始分组操作
/// @param [in] group       分组名称
/// @return XPR_TRUE or XPR_FALSE
XPR_API int XPR_UPS_BeginGroup(const char* group);

/// @brief 结束分组操作
/// @param [in] group       分组名称
XPR_API void XPR_UPS_EndGroup(const char* group);

/// 对指定前缀的所有项设定 shadow 位
/// @note 设定 shadow 位后，调用 SetXXX 接口设置的数据将会缓存起来，
///       并且会跳过 setter 回调
XPR_API void XPR_UPS_Shadow(const char* prefix);

/// 对指定前缀的所有项清除 shadow 位
/// @note 清除 shadow 位后，会利用之前缓存的数据传递给 setter 使用
XPR_API void XPR_UPS_Expose(const char* prefix);

/// @brief 导出数据为平坦文件
/// @param [in] fileName    平坦文件路径
/// @retval 见错误码表
XPR_API int XPR_UPS_Export(const char* fileName);

/// @brief 从平坦文件导入数据
/// @param [in] url         平坦文件路径
/// @retval 见错误码表
XPR_API int XPR_UPS_Import(const char* url);

/// @brief 压实数据文件
/// @retval 见错误码表
XPR_API int XPR_UPS_Pack(void);

/// 打印所有已注册的设定项到标准输出
/// @retval 见错误码表
XPR_API int XPR_UPS_PrintAll(void);

/// @brief 同步数据到存储介质
/// @retval 见错误码表
XPR_API int XPR_UPS_Sync(void);

#ifdef __cplusplus
}
#endif

#endif // XPR_UPS_H
