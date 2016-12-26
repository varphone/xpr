#ifndef XPR_JSON_H
#define XPR_JSON_H

/// @defgroup xprdjsonc JSON
/// @brief     采用 C 语言规范编写的一套接口, 用于操作基于 JavaScript 语言的轻量级的数据交换格式
/// @author    Varphone Wong [varphone@163.com]
/// @version   1.6.1
/// @date      2015/4/16
///
/// @{
///

#include <stddef.h> // for size_t;
#include <stdint.h> // for int64_t;
#include <stdio.h> // for FILE*;
#include <xpr/xpr_common.h> // for XPR_API;

/// @page xprjsonc-changes 变更日志
///
/// @par 1.6.1 (2015/4/16)
///   - 增加 XPR_JSON_ObjectRemove(), XPR_JSON_ObjectClear(), XPR_JSON_ArrayRemove(), XPR_JSON_ArrayClear()
///
/// @par 1.2.1 (2015/2/3)
///   - 增加 XPR_JSON_Copy(), XPR_JSON_DeepCopy() 接口
///
/// @par 1.1.1 (2013/7/30)
///   - 增加 XPR_JSON_RefCount() 用于获取被引用的数量
///
/// @par 1.1.0 (2013/6/24)
///   - 修改 XPR_JSON_UNDEFINED 值为 -1, 应用程序需要重新编译
///
/// @par 1.0.2 (2013/6/19)
///   - 增加 XPR_JSON_Integer64(), XPR_JSON_Integer64Set(), XPR_JSON_Integer64Value() 64 位整数支持接口
///
/// @par 1.0.1 (2013/4/8)
///   - 增加 #LIBXPR_JSON_VERSION
///   - 增加 libXPR_JSON_Version()
///   - 增加 libXPR_JSON_VersionNumber()
///
/// @par 1.0 (2012/12/20)
///   - 初始本版建立
///
///

///
/// 当前定义版本号
///
#define XPR_JSON_VERSION XPR_MakeVersion(1,6,1)

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef XPR_JSON_TYPE_DEFINED
#define XPR_JSON_TYPE_DEFINED
///
/// JSON 对象数据结构前置声明
///
struct XPR_JSON;
///
/// JSON 对象数据类型声明
///
typedef struct XPR_JSON XPR_JSON;
#endif // XPR_JSON_TYPE_DEFINED

#ifndef XPR_JSON_TYPE_TYPE_DEFINED
#define XPR_JSON_TYPE_TYPE_DEFINED
///
/// JSON 数据类型
///
typedef enum XPR_JSON_TYPE {
    XPR_JSON_UNDEFINED = -1,    ///< 未定义
    XPR_JSON_OBJECT = 0,     ///< 键值对
    XPR_JSON_ARRAY,      ///< 数组
    XPR_JSON_STRING,     ///< 字符串
    XPR_JSON_INTEGER,    ///< 整数
    XPR_JSON_REAL,       ///< 实数(浮点数)
    XPR_JSON_TRUE,       ///< 布尔(真)
    XPR_JSON_FALSE,      ///< 布尔(假)
    XPR_JSON_NULL,        ///< 空值
} XPR_JSON_TYPE;
#endif // XPR_JSON_TYPE_TYPE_DEFINED

/// @brief 获取 XPR_JSON 库版本信息
/// @return 包含版本信息的字符串
XPR_API const char* XPR_JSON_Version(void);

/// @brief 获取 XPR_JSON 库版本号
/// @return 返回整数形式的版本号数字
/// @remark 版本号说明
///   数据位  | 说明
///   -------|----------
///   31~24  | 主版本号
///   23~16  | 子版本号
///   15~00  | 修正版本号
///
XPR_API int XPR_JSON_VersionNumber(void);

/// @brief 分配内存
/// @param [in] size    分配的内存大小
/// @retval NULL        分配失败
/// @retval !NULL       分配到的内存地址
/// @note 当返回值不再使用时，需调用 [#XPR_JSON_Free] 释放资源
/// @note 线程安全
XPR_API void* XPR_JSON_Alloc(size_t size);

/// @brief 释放由 #XPR_JSON_Alloc 分配的内存
/// @param [in] ptr     要释放的内存地址, 参见 #XPR_JSON_Alloc
/// @return 无返回值
/// @note 线程安全
XPR_API void XPR_JSON_Free(void* ptr);

/// @brief 增加引用
///        XPR_JSON 对象的资源管理采用引用计数方式来实现, 所有新对象创建的初始引用计数都为: 1
/// @param [in] json    #XPR_JSON 实例
/// @return #XPR_JSON 实例
/// @sa XPR_JSON_DecRef()
/// @warning 非线程安全
XPR_API XPR_JSON* XPR_JSON_IncRef(XPR_JSON* json);

/// @brief 减少引用
///        当引用计数为 0 时, 对象将会被销毁
/// @param [in] json    #XPR_JSON 实例
/// @return 无返回值
/// @sa XPR_JSON_IncRef()
/// @warning 非线程安全
XPR_API void XPR_JSON_DecRef(XPR_JSON* json);

/// @brief 获取对象的引用计数值
/// @param [in] json    #XPR_JSON 实例
/// @retval -1  非法对象
/// @retval 0   对象已经释放
/// @retval >0  对象被引用的数量
XPR_API int XPR_JSON_RefCount(XPR_JSON* json);

/// @brief 将字符串类型的 JSON 文本转成 #XPR_JSON 类型
/// @param [in] text    需要转换的值
/// @retval NULL    转换失败
/// @retval !NULL   #XPR_JSON 实例
XPR_API XPR_JSON* XPR_JSON_LoadString(const char* text);

/// @brief 从文件载入 JSON 数据
/// @param [in] fileName    JSON 文件路径
/// @retval NULL    转换失败
/// @retval !NULL   #XPR_JSON 实例
XPR_API XPR_JSON* XPR_JSON_LoadFileName(const char* fileName);

/// @brief 将 #XPR_JSON 对象转换成字符串格式
/// @param [in] json    #XPR_JSON 实例
/// @return 文本格式的 JSON 字符串指针
/// @warning 非线程安全
XPR_API char* XPR_JSON_DumpString(XPR_JSON* json);

/// @brief 转储到指定文件中
/// @param [in] json        要操作的 #XPR_JSON 实例
/// @param [in] fileName    文件名
/// @retval 0   成功
/// @retval -1  失败
/// @warning 非线程安全
XPR_API int XPR_JSON_DumpFileName(XPR_JSON* json, const char* fileName);

/// @brief 转储到文件流中
/// @param [in] json        要操作的 #XPR_JSON 实例
/// @param [in] fileStream  已打开的文件流, 见 [fopen, fopen_s]
/// @retval 0   成功
/// @retval -1  失败
/// @warning 非线程安全
XPR_API int XPR_JSON_DumpFileStream(XPR_JSON* json, FILE* fileStream);

/// @brief 获取 #XPR_JSON 对象的类型
/// @param [in] json    #XPR_JSON 实例
/// @return json 的类型，参见 #XPR_JSON_TYPE
/// @note 线程安全
XPR_API XPR_JSON_TYPE XPR_JSON_Typeof(XPR_JSON* json);

/// @brief 检查是否是 Object 类型
/// @param [in] json    #XPR_JSON 实例
/// @retval 0   成功
/// @retval -1  失败
/// @note 线程安全
XPR_API int XPR_JSON_IsObject(XPR_JSON* json);

/// @brief 检查是否是Array类型
/// @param [in] json    #XPR_JSON 实例
/// @retval 0   成功
/// @retval -1  失败
/// @note 线程安全
XPR_API int XPR_JSON_IsArray(XPR_JSON* json);

/// @brief 检查是否是 String 类型
/// @param [in] json    #XPR_JSON 实例
/// @retval 0   成功
/// @retval -1  失败
/// @note 线程安全
XPR_API int XPR_JSON_IsString(XPR_JSON* json);

/// @brief 检查是否是 Integer 类型
/// @param [in] json    #XPR_JSON 实例
/// @retval 0   成功
/// @retval -1  失败
/// @note 线程安全
XPR_API int XPR_JSON_IsInteger(XPR_JSON* json);

/// @brief 检查是否是 Real 类型
/// @param [in] json    #XPR_JSON 实例
/// @retval 0   成功
/// @retval -1  失败
/// @note 线程安全
XPR_API int XPR_JSON_IsReal(XPR_JSON* json);

/// @brief 检查是否是 True 类型
/// @param [in] json    #XPR_JSON 实例
/// @retval 0   成功
/// @retval -1  失败
/// @note 线程安全
XPR_API int XPR_JSON_IsTrue(XPR_JSON* json);

/// @brief 检查是否是 False 类型
/// @param [in] json    #XPR_JSON 实例
/// @retval 0   成功
/// @retval -1  失败
/// @note 线程安全
XPR_API int XPR_JSON_IsFalse(XPR_JSON* json);

/// @brief 检查是否是 Null 类型
/// @param [in] json    #XPR_JSON 实例
/// @retval 0   成功
/// @retval -1  失败
/// @note 线程安全
XPR_API int XPR_JSON_IsNull(XPR_JSON* json);

/// @brief 创建一个 Object 对象
/// @retval NULL    创建失败
/// @retval !NULL   #XPR_JSON 实例
/// @note 线程安全
XPR_API XPR_JSON* XPR_JSON_Object(void);

/// @brief 往 Object 类型的 JSON 对象中追加成员(引用方式)
/// @param [in] json    #XPR_JSON 对象实例(#XPR_JSON_OBJECT)
/// @param [in] key     键名
/// @param [in] val     #XPR_JSON 对象实例(Any)
/// @retval 0   成功
/// @retval -1  失败
/// @note 本接口会增加对 val 的引用，因此在调用此接口调用后且此对象不再使用时，需要调用 #XPR_JSON_DecRef 释放对象
/// @sa XPR_JSON_Object(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API int XPR_JSON_ObjectSet(XPR_JSON* json, const char* key,
				               XPR_JSON* val);

/// @brief 往 Object 类型的 JSON 对象中追加成员(借用方式)
/// @param [in] json    #XPR_JSON 对象实例(#XPR_JSON_OBJECT)
/// @param [in] key     键名
/// @param [in] val     #XPR_JSON 对象实例(Any)
/// @retval 0   成功
/// @retval -1  失败
/// @note 本接口不会增加对 val 的引用，因此在调用此接口调用后切勿使用 #XPR_JSON_DecRef 释放对象，否则可能导致异常出现
/// @sa XPR_JSON_Object(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API int XPR_JSON_ObjectSetNew(XPR_JSON* json, const char* key,
				                  XPR_JSON* val);

/// @brief 获取 Object 对象中的值
/// @param [in] json    #XPR_JSON 实例
/// @param [in] key     要设置的值
/// @return #XPR_JSON 实例
/// @sa XPR_JSON_Object(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API XPR_JSON* XPR_JSON_ObjectGet(XPR_JSON* json, const char* key);

/// @brief 获取 Object 对象的大小
/// @param [in] json    #XPR_JSON 实例
/// @retval 0   空或对象不存在
/// @retval >0  Object 对象大小
/// @sa XPR_JSON_Object(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API size_t XPR_JSON_ObjectSize(XPR_JSON* json);

///
/// 移除指定键值的成员对象
///
/// @param [in] json    #XPR_JSON 实例
/// @param [in] key     要移除的成员的对象的键值
/// @retval 0   成功
/// @retval -1  失败
XPR_API int XPR_JSON_ObjectRemove(XPR_JSON* json, const char* key);

///
/// 清除所成员对象
///
/// @param [in] json    #XPR_JSON 实例
/// @retval 0   成功
/// @retval -1  失败
XPR_API int XPR_JSON_ObjectClear(XPR_JSON* json);

/// @brief 创建一个迭代器
/// @param [in] json    #XPR_JSON 实例
/// @retval NULL    创建失败
/// @retval !NULL   迭代器实例
/// @sa XPR_JSON_Object(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API void* XPR_JSON_ObjectIter(XPR_JSON* json);

/// @brief 获取 json 中指定键值所在的迭代器
/// @param [in] json    #XPR_JSON 实例
/// @param [in] key     键名
/// @retval NULL    获取失败
/// @retval !NULL   迭代器实例
/// @sa XPR_JSON_Object(), XPR_JSON_ObjectIter(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API void* XPR_JSON_ObjectIterAt(XPR_JSON* json, const char* key);

/// @brief 将迭代器的指针移到下一个位置
/// @param [in] json    #XPR_JSON 实例
/// @param [in] iter    迭代器实例
/// @retval NULL    获取失败
/// @retval !NULL   迭代器实例
/// @sa XPR_JSON_Object(), XPR_JSON_ObjectIter(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API void* XPR_JSON_ObjectIterNext(XPR_JSON* json, void* iter);

/// @brief 获取迭代器所指向对象的键名
/// @param [in] iter    迭代器实例
/// @retval NULL    获取失败
/// @retval !NULL   对象键名
/// @sa XPR_JSON_Object(), XPR_JSON_ObjectIter(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API const char* XPR_JSON_ObjectIterKey(void* iter);

/// @brief 获取迭代器所指对象的键值
/// @param [in] iter    迭代器实例
/// @retval NULL    获取失败
/// @retval !NULL   #XPR_JSON 实例
/// @sa XPR_JSON_Object(), XPR_JSON_ObjectIter(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API XPR_JSON* XPR_JSON_ObjectIterValue(void* iter);

/// @brief 往 迭代器 中追加成员(引用方式)
/// @param [in] json    #XPR_JSON 对象实例
/// @param [in] iter    迭代器
/// @param [in] val     #XPR_JSON 对象实例(Any)
/// @retval 0   成功
/// @retval -1  失败
/// @note 本接口会增加对 val 的引用，因此在调用此接口调用后且此对象不再使用时，需要调用 #XPR_JSON_DecRef 释放对象
/// @sa XPR_JSON_Object(), XPR_JSON_ObjectIter(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API int XPR_JSON_ObjectIterSet(XPR_JSON* json, void* iter, XPR_JSON* val);

/// @brief 往迭代器中追加成员(借用方式)
/// @param [in] json    #XPR_JSON 对象实例
/// @param [in] iter    迭代器
/// @param [in] val     #XPR_JSON 对象实例(Any)
/// @retval 0   成功
/// @retval -1  失败
/// @note 本接口不会增加对 val 的引用，因此在调用此接口调用后切勿使用 #XPR_JSON_DecRef 释放对象，否则可能导致异常出现
/// @sa XPR_JSON_Object(), XPR_JSON_ObjectIter(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API int XPR_JSON_ObjectIterSetNew(XPR_JSON* json, void* iter,
                              XPR_JSON* val);

/// @brief 创建一个 Array 类型的 JSON 对象
/// @retval NULL    创建失败
/// @retval !NULL   #XPR_JSON 实例
/// @note 线程安全
XPR_API XPR_JSON* XPR_JSON_Array(void);

/// @brief 设置 Array 对象中的值
/// @param [in] json    要操作的 #XPR_JSON 实例
/// @param [in] index   Array 对象的索引
/// @param [in] val     要设置的值
/// @retval 0   成功
/// @retval -1  失败
/// @note 本接口会增加对 val 的引用，因此在调用此接口调用后且此对象不再使用时，需要调用 #XPR_JSON_DecRef 释放对象
/// @sa XPR_JSON_Array(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API int XPR_JSON_ArraySet(XPR_JSON* json, size_t index, XPR_JSON* val);

/// @brief 设置 Array 对象中的值
/// @param [in] json    要操作的 #XPR_JSON 实例
/// @param [in] index   Array 对象的索引
/// @param [in] val     要设置的值
/// @retval 0   成功
/// @retval -1  失败
/// @note 本接口不会增加对 val 的引用，因此在调用此接口调用后切勿使用 #XPR_JSON_DecRef 释放对象，否则可能导致异常出现
/// @sa XPR_JSON_Array(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API int XPR_JSON_ArraySetNew(XPR_JSON* json, size_t index, XPR_JSON* val);

/// @brief 获取 Array 对象中的值
/// @param [out] json   要操作的 #XPR_JSON 实例
/// @param [in] index   数组的索引
/// @return #XPR_JSON 实例
/// @sa XPR_JSON_Array(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API XPR_JSON* XPR_JSON_ArrayGet(XPR_JSON* json, size_t index);

/// @brief 获取 Array 对象的大小
/// @param [in] json    #XPR_JSON 实例
/// @return 返回数组中元素的数量，如果数组为空返回 0
/// @sa XPR_JSON_Array(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API size_t XPR_JSON_ArraySize(XPR_JSON* json);

/// @brief 往 Array 类型的 JSON 对象中追加成员(引用方式)
/// @param [in] json    #XPR_JSON 对象实例(#XPR_JSON_ARRAY)
/// @param [in] val     #XPR_JSON 对象实例(Any)
/// @retval 0   成功
/// @retval -1  失败
/// @note 本接口会增加对 val 的引用，因此在调用此接口调用后且此对象不再使用时，需要调用 #XPR_JSON_DecRef 释放对象
/// @code
/// XPR_JSON* js = XPR_JSON_String("Hello, JSON");
/// XPR_JSON* ja = XPR_JSON_Array();
/// // 追加 js 对象到 ja 数组中
/// XPR_JSON_ArrayAppend(ja, js);
/// ...
/// // 释放 ja 数组对象
/// XPR_JSON_DecRef(ja);
/// // 释放 js 字符串对象
/// XPR_JSON_DecRef(js);
/// @endcode
/// @sa XPR_JSON_Array(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API int XPR_JSON_ArrayAppend(XPR_JSON* json, XPR_JSON* val);

/// @brief 往 Array 类型的 JSON 对象中追加成员(借用方式)
/// @param [in] json    #XPR_JSON 对象实例(#XPR_JSON_ARRAY)
/// @param [in] val     #XPR_JSON 对象实例(Any)
/// @retval 0   成功
/// @retval -1  失败
/// @note 本接口不会增加对 val 的引用，因此在调用此接口调用后切勿使用 #XPR_JSON_DecRef 释放对象，否则可能导致异常出现
/// @code
/// XPR_JSON* js = XPR_JSON_String("Hello, JSON");
/// XPR_JSON* ja = XPR_JSON_Array();
/// // 追加 js 对象到 ja 数组中
/// XPR_JSON_ArrayAppendNew(ja, js);
/// ...
/// // 释放 ja 数组对象
/// XPR_JSON_DecRef(ja);
/// // 因为 js 对象以借用方式追加到 ja 数组对象中，引用计数并没增加
/// // 当 ja 释放时会自动释放所有关联对象，因此 js 不需要单独释放
/// //XPR_JSON_DecRef(js);
/// @endcode
/// @sa XPR_JSON_Array(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API int XPR_JSON_ArrayAppendNew(XPR_JSON* json, XPR_JSON* val);

/// @brief  往 Array 类型的 JSON 对象中插入成员(借用方式)
///
XPR_API int XPR_JSON_ArrayInsertNew(XPR_JSON* json, size_t index, XPR_JSON* val);

///
/// 移除指定索引的对象
///
/// @param [in] json    #XPR_JSON 对象实例(#XPR_JSON_ARRAY)
/// @param [in] index   要移除的子对象的索引
/// @retval 0   成功
/// @retval -1  失败
/// @note 本操作会改变数组的大小
XPR_API int XPR_JSON_ArrayRemove(XPR_JSON* json, size_t index);

///
/// 清除所有成员对象
///
/// @param [in] json    #XPR_JSON 对象实例(#XPR_JSON_ARRAY)
/// @retval 0   成功
/// @retval -1  失败
/// @note 本操作会改变数组的大小
XPR_API int XPR_JSON_ArrayClear(XPR_JSON* json);

/// @brief 创建一个 String 对象
/// @param [in] val     对象的初始值
/// @return #XPR_JSON 实例
/// @note 线程安全
XPR_API XPR_JSON* XPR_JSON_String(const char* val);

/// @brief 设置 String 对象中的值
/// @param [in] json    要操作的 #XPR_JSON 实例
/// @param [in] val     要设置的字符串
/// @retval 0   成功
/// @retval -1  失败
/// @sa XPR_JSON_String(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API int XPR_JSON_StringSet(XPR_JSON* json, const char* val);

/// @brief 获取 String 对象中的值
/// @param [in] json    #XPR_JSON 实例
/// @retval NULL    对象不能存在或类型不匹配
/// @retval !NULL   对象关联的字符串指针
/// @sa XPR_JSON_String(), XPR_JSON_LoadString()
/// @warning 返回值指针所指向的内存资源会随着 json 对象释放而释放，因此如果需要长期保存返回值内容请保存副本而非保存指针引用
/// @warning 非线程安全
XPR_API const char* XPR_JSON_StringValue(XPR_JSON* json);

/// @brief 创建一个 Interger 对象
/// @param [in] val     对象初始值
/// @retval NULL    创建失败
/// @retval !NULL   #XPR_JSON 实例
XPR_API XPR_JSON* XPR_JSON_Integer(int val);

/// @brief 创建一个 Interger(64位) 对象
/// @param [in] val     对象初始值
/// @retval NULL    创建失败
/// @retval !NULL   #XPR_JSON 实例
XPR_API XPR_JSON* XPR_JSON_Integer64(int64_t val);

/// @brief 设置 Interger 对象中的值
/// @param [in] json    #XPR_JSON 实例
/// @param [in] val     要设置的整型值
/// @retval 0   成功
/// @retval -1  失败
/// @sa XPR_JSON_Integer(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API int XPR_JSON_IntegerSet(XPR_JSON* json, int val);

/// @brief 设置 Interger(64位) 对象中的值
/// @param [in] json    #XPR_JSON 实例
/// @param [in] val     要设置的整型值
/// @retval 0   成功
/// @retval -1  失败
/// @sa XPR_JSON_Integer(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API int XPR_JSON_Integer64Set(XPR_JSON* json, int64_t val);

/// @brief 获取 Interger 对象中的值
/// @param [in] json    #XPR_JSON 实例
/// @return 返回相关值真实, 如果不是一个 XPR_JSON 实例返回 0
/// @sa XPR_JSON_Integer(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API int XPR_JSON_IntegerValue(XPR_JSON* json);

/// @brief 获取 Interger 对象中的值
/// @param [in] json    #XPR_JSON 实例
/// @return 返回相关值真实, 如果不是一个 XPR_JSON 实例返回 0
/// @sa XPR_JSON_Integer(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API int64_t XPR_JSON_Integer64Value(XPR_JSON* json);

/// @brief 创建一个 Real 对象
/// @param [in] val     对象的初始值
/// @retval NULL    创建失败
/// @retval !NULL   #XPR_JSON 实例
XPR_API XPR_JSON* XPR_JSON_Real(double val);

/// @brief 设置 Real 对象中的值
/// @param [in] json    #XPR_JSON 实例
/// @param [in] val     对象初始值
/// @retval 0   成功
/// @retval -1  失败
/// @sa XPR_JSON_Real(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API int XPR_JSON_RealSet(XPR_JSON* json, double val);

/// @brief 获取 Real 对象的值
/// @param [in] json    #XPR_JSON 实例
/// @return 返回相关值真实, 如果不是一个 XPR_JSON 实例返回 0.0
/// @sa XPR_JSON_Real(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API double XPR_JSON_RealValue(XPR_JSON* json);

/// @brief 创建一个 bool 值为真的对象
/// @return #XPR_JSON 实例
/// @note 线程安全
XPR_API XPR_JSON* XPR_JSON_True(void);

/// @brief 取 #XPR_JSON_True() 对象的值
/// @param [in] json    #XPR_JSON 实例
/// @retval 0   Not True
/// @retval !0  True
/// @sa XPR_JSON_Boolean(), XPR_JSON_True(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API int XPR_JSON_TrueValue(XPR_JSON* json);

/// @brief 创建一个 Boolean(False) 对象
/// @retval NULL    创建失败
/// @retval !NULL   #XPR_JSON 实例
/// @note 线程安全
XPR_API XPR_JSON* XPR_JSON_False(void);

/// @brief 取 #XPR_JSON_False() 对象中的值
/// @param [in] json    #XPR_JSON 实例
/// @retval 0   False
/// @retval !0  Not False
/// @sa XPR_JSON_Boolean(), XPR_JSON_False(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API int XPR_JSON_FalseValue(XPR_JSON* json);

/// @brief 创建一个 Boolean(True/False) 对象
/// @param [in] val     对象初始值
/// @retval NULL    创建失败
/// @retval !NULL   #XPR_JSON 实例
/// @note 线程安全
XPR_API XPR_JSON* XPR_JSON_Boolean(int val);

/// @brief 取 #XPR_JSON_Boolean() 对象中的值
/// @param [in] json    #XPR_JSON 实例
/// @retval 0   False
/// @retval !0  True
/// @sa XPR_JSON_Boolean(), XPR_JSON_True(), XPR_JSON_False(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API int XPR_JSON_BooleanValue(XPR_JSON* json);

/// @brief 创建一个空对象
/// @retval NULL    创建失败
/// @retval !NULL   #XPR_JSON 实例
/// @note 线程安全
XPR_API XPR_JSON* XPR_JSON_Null(void);

/// @brief 取空对象中的值
/// @param [in] json    #XPR_JSON 实例
/// @return NULL
/// @sa XPR_JSON_Null(), XPR_JSON_LoadString()
/// @warning 非线程安全
XPR_API void* XPR_JSON_NullValue(XPR_JSON* json);

///
/// 复制一个 XPR_JSON 对象
///
/// @param [in] src   要复制的　#XPR_JSON 实例
/// @return 返回新的 XPR_JSON 实例
XPR_API XPR_JSON* XPR_JSON_Copy(XPR_JSON* src);

///
/// 复制一个 XPR_JSON 对象及其所包含的子对象
///
/// @param [in] src   要复制的　#XPR_JSON 实例
/// @return 返回新的 XPR_JSON 实例
XPR_API XPR_JSON* XPR_JSON_DeepCopy(XPR_JSON* src);

/// @brief 以 XPath 方式获取 JSON 实例
/// @param [in] json	目标根（父）节点
/// @param [in] xpath	XPath 参数
/// @code
/// XPR_JSON* root = XPR_JSON_LoadString(...);
/// // 获取 Object 属性
/// XPR_JSON* jx = XPR_JSON_XPathGet(root, "/top/sub/attr1");
/// // 获取 Array 指定索引
/// XPR_JSON* jz = XPR_JSON_XPathGet(root, "/top/sub/attrs[1]");
/// @endcode
XPR_API XPR_JSON* XPR_JSON_XPathGet(XPR_JSON* json, const char* xpath);

/// @brief 以 XPath 方式设置键值（引用）
/// @note 本接口会增加 `value` 的引用计数，如果你在调用本接口后，如果没有对 `value` 调用 XPR_JSON_DecRef()，可能会导致内存泄漏。
XPR_API int XPR_JSON_XPathSet(XPR_JSON* json, const char* xpath, XPR_JSON* value);

/// @brief 以 XPath 方式设置键值（借用）
/// @note 本接口不会增加 `value` 的引用计数。
XPR_API int XPR_JSON_XPathSetNew(XPR_JSON* json, const char* xpath, XPR_JSON* value);

/// @brief 检测 XPath 对应的对象是否为 Null。
/// @return true/false
XPR_API int XPR_JSON_XPathIsNull(XPR_JSON* json, const char* xpath);

/// @brief 以 XPath 方式直接获取布尔值
/// @param [in] json	目标根（父）节点
/// @param [in] xpath	XPath 参数
/// @retval 0 false
/// @retval 1 true
XPR_API int XPR_JSON_XPathGetBoolean(XPR_JSON* json, const char* xpath);

/// @brief 以 XPath 方式直接设置布尔值
/// @see XPR_JSON_XPathGetBoolean()
/// @return XPR_ERR_xxx
XPR_API int XPR_JSON_XPathSetBoolean(XPR_JSON* json, const char* xpath, int value);

/// @brief 以 XPath 方式直接获取整形值（32位）
/// @param [in] json	目标根（父）节点
/// @param [in] xpath	XPath 参数
/// @retval 0 对象不存在或其值为 0
/// @retval other 对象的值
XPR_API int XPR_JSON_XPathGetInt(XPR_JSON* json, const char* xpath);

/// @brief 以 XPath 方式直接设置整形值（32位）
/// @return XPR_ERR_xxx
/// @see XPR_JSON_XPathGetInt()
XPR_API int XPR_JSON_XPathSetInt(XPR_JSON* json, const char* xpath, int value);

/// @brief 以 XPath 方式直接获取整形值（64位）
/// @param [in] json	目标根（父）节点
/// @param [in] xpath	XPath 参数
/// @retval 0 对象不存在或其值为 0
/// @retval other 对象的值
XPR_API int64_t XPR_JSON_XPathGetInt64(XPR_JSON* json, const char* xpath);

/// @brief 以 XPath 方式直接设置整形值（64位）
/// @return XPR_ERR_xxx
/// @see XPR_JSON_XPathGetInt64()
XPR_API int XPR_JSON_XPathSetInt64(XPR_JSON* json, const char* xpath, int64_t value);

/// @brief 以 XPath 方式直接获取浮点值（32位）
/// @param [in] json	目标根（父）节点
/// @param [in] xpath	XPath 参数
/// @retval 0.0 对象不存在或其值为 0.0
/// @retval other 对象的值
XPR_API float XPR_JSON_XPathGetFloat(XPR_JSON* json, const char* xpath);

/// @brief 以 XPath 方式直接设置浮点值（32位）
/// @return XPR_ERR_xxx
/// @see XPR_JSON_XPathGetFloat()
XPR_API int XPR_JSON_XPathSetFloat(XPR_JSON* json, const char* xpath, float value);

/// @brief 以 XPath 方式直接获取浮点值（64位）
/// @param [in] json	目标根（父）节点
/// @param [in] xpath	XPath 参数
/// @retval 0.0 对象不存在或其值为 0.0
/// @retval other 对象的值
XPR_API double XPR_JSON_XPathGetDouble(XPR_JSON* json, const char* xpath);

/// @brief 以 XPath 方式直接设置浮点值（32位）
/// @return XPR_ERR_xxx
/// @see XPR_JSON_XPathGetDouble()
XPR_API int XPR_JSON_XPathSetDouble(XPR_JSON* json, const char* xpath, double value);

/// @brief 以 XPath 方式获取数值。
/// @note 本接口会自动将 True/False, Int/Int64, Number String 转换为 double 型值。
///       例如：True = 1.0， False = 0.0， 111 = 111.0， "123" = 123.0
XPR_API double XPR_JSON_XPathGetNumber(XPR_JSON* json, const char* xpath);

// XPR_JSON_XPathSetNumber() 不支持。

/// @brief 以 XPath 方式直接获取字符串值
/// @param [in] json	目标根（父）节点
/// @param [in] xpath	XPath 参数
/// @retval NULL 对象不存在或其值为 NULL
/// @retval other 对象的值
XPR_API const char* XPR_JSON_XPathGetString(XPR_JSON* json, const char* xpath);

/// @brief 以 XPath 方式直接设置字符串值
/// @return XPR_ERR_xxx
/// @see XPR_JSON_XPathGetString()
XPR_API int XPR_JSON_XPathSetString(XPR_JSON* json, const char* xpath, const char* value);

#ifdef __cplusplus
}
#endif

/// @}
///

#endif // XPR_JSON_H

