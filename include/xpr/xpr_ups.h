#ifndef XPR_UPS_H
#define XPR_UPS_H

#include <stdint.h>
#include "xpr_common.h"
#include "xpr_json.h"
#include "xpr_errno.h"

/// @defgroup xprups 设定集操作库
/// @brief     Universal Preference Settings Foundation library.
/// @author    Varphone Wong [varphone@163.com]
/// @version   1.0.0
/// @date      2013/4/1
///
/// @{
///

/// @defgroup xprups-changes 变更日志
/// @{
///
/// @par 1.0.0 (2012/12/20)
///   - 初始版本建立
///
/// @}
///

#ifdef __cplusplus
extern "C"
{
#endif

/// @addtogroup xprups-macros 宏定义
/// @{
///

#define XPR_UPS_VERSION  XPR_MakeVersion(1,0,0)

/// @}
///

#ifndef XPR_UPS_ENTRY_TYPE_DEFINED
#define XPR_UPS_ENTRY_TYPE_DEFINED
struct XPR_UPS_Entry; ///< XPR_UPS 前置声明
typedef struct XPR_UPS_Entry XPR_UPS_Entry; ///< XPR_UPS 类型声明
#endif // XPR_UPS_ENTRY_TYPE_DEFINED


#ifndef XPR_UPS_ENTRYTYPE_TYPE_DEFINED
#define XPR_UPS_ENTRYTYPE_TYPE_DEFINED
/// @brief XPR_UPS 条目类型
enum XPR_UPS_EntryType {
    XPR_UPS_ENTRY_TYPE_UNKNOWN, ///< 未知
    XPR_UPS_ENTRY_TYPE_DIR,     ///< 目录
    XPR_UPS_ENTRY_TYPE_BOOLEAN, ///< 布尔
    XPR_UPS_ENTRY_TYPE_BLOB,    ///< 二进制数据
    XPR_UPS_ENTRY_TYPE_INT,     ///< 整形
    XPR_UPS_ENTRY_TYPE_INT64,   ///< 整形(64位)
    XPR_UPS_ENTRY_TYPE_REAL,    ///< 实数
    XPR_UPS_ENTRY_TYPE_STRING,  ///< 字符串
};
typedef enum XPR_UPS_EntryType XPR_UPS_EntryType; ///< XPR_UPS 条目类型
#endif // XPR_UPS_ENTRYTYPE_TYPE_DEFINED

/// @brief 设定项初始化函数
/// @param [in] ent     设定项指针
/// @return see [#XPR_ErrorCode]
typedef int (*XPR_UPS_Initlializer)(XPR_UPS_Entry* ent);
typedef int (*XPR_UPS_Finalizer)(XPR_UPS_Entry* ent);
typedef int (*XPR_UPS_Getter)(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key, void* buffer, int* size);
typedef int (*XPR_UPS_Setter)(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key, const void* data, int size);

struct XPR_UPS_Entry {
    const char** names;
    const char** descs;
    const char* category;
    const char* root;
    XPR_UPS_EntryType type;
    XPR_UPS_Initlializer init;
    XPR_UPS_Finalizer fini;
    XPR_UPS_Getter get;
    XPR_UPS_Setter set;
    XPR_UPS_Entry* prev;
    XPR_UPS_Entry* next;
    XPR_UPS_Entry* subs;
};

#define XPR_UPS_ENTRY_END   { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }



/// 调用成功
#define XPR_ERR_UPS_SUCCESS          0
 
/// @brief Error code define
/// 传进去的参数无效
#define XPR_ERR_UPS_ILLEGAL_PARAM    XPR_DEF_ERR(XPR_ID_UPS, EN_ERR_LEVEL_ERROR, EN_ERR_ILLEGAL_PARAM)
/// 资源已存在
#define XPR_ERR_UPS_EXISTED          XPR_DEF_ERR(XPR_ID_UPS, EN_ERR_LEVEL_ERROR, EN_ERR_EXIST)
/// 资源不存在
#define XPR_ERR_UPS_UNEXIST          XPR_DEF_ERR(XPR_ID_UPS, EN_ERR_LEVEL_ERROR, EN_ERR_UNEXIST)
/// 指针为空
#define XPR_ERR_UPS_NULL_PTR         XPR_DEF_ERR(XPR_ID_UPS, EN_ERR_LEVEL_ERROR, EN_ERR_NULL_PTR)
/// 系统未配置
#define XPR_ERR_UPS_NOT_CONFIG       XPR_DEF_ERR(XPR_ID_UPS, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_CONFIG)
/// 当前操作不支持
#define XPR_ERR_UPS_NOT_SUPPORT      XPR_DEF_ERR(XPR_ID_UPS, EN_ERR_LEVEL_ERROR, EN_ERR_NOT_SUPPORT)
/// 不允许操作
#define XPR_ERR_UPS_NOT_PERM         XPR_DEF_ERR(XPR_ID_UPS, EN_ERR_LEVEL_ERROR,  EN_ERR_NOT_PERM)
/// 内薰申请失败
#define XPR_ERR_UPS_NOMEM            XPR_DEF_ERR(XPR_ID_UPS, EN_ERR_LEVEL_ERROR, EN_ERR_NOMEM)
/// 缓冲区太小
#define XPR_ERR_UPS_NOBUF            XPR_DEF_ERR(XPR_ID_UPS, EN_ERR_LEVEL_ERROR,  EN_ERR_NOBUF)
/// UPS未初始化
#define XPR_ERR_UPS_NOTREADY         XPR_DEF_ERR(XPR_ID_UPS, EN_ERR_LEVEL_ERROR,  EN_ERR_SYS_NOTREADY)


/// @brief 打开设定集
/// @retval <=0 失败
/// @retval >0  已打开的设定集实例
int XPR_UPS_Init(void);

/// @brief 关闭已打开的设定集
/// @retval -1  失败
/// @retval 0   成功
int XPR_UPS_Fini(void);

/// @brief 注册UPS节点
/// @param [in] ents 需要添加的节点数组
/// @param [in] ecount 需要添加的节点数
/// @retval 见错误码表
int XPR_UPS_Register(XPR_UPS_Entry ents[], int count);

/// @brief 卸载UPS节点
/// @param [in] ents 需要卸载的节点数组
/// @param [in] ecount 需要卸载的节点数
int XPR_UPS_UnRegister(XPR_UPS_Entry ents[], int count);

/// @brief 设置字串型设定项值
/// @param [in] key         设定项名称
/// @param [in] value       设定项值
/// @retval 见错误码表
int XPR_UPS_SetString(const char* key, const char* value, int size);
int XPR_UPS_SetStringVK(const char* value, int size, const char* key, ...);

/// @brief 获取字串型设定项值
/// @param [in] key         设定项名称
/// @retval NULL    设定项不存在或设定项值为空
/// @retval 见错误码表
int XPR_UPS_GetString(const char* key, char* value, int* size);
int XPR_UPS_GetStringVK(char* value, int* size, const char* key, ...);

/// @brief 设置整数型设定项值
/// @param [in] key         设定项名称
/// @param [in] value       设定项值
/// @retval 见错误码表
int XPR_UPS_SetInteger(const char* key, int value);
int XPR_UPS_SetIntegerVK(int value, const char* key, ...);

/// @brief 获取整数型设定项值
/// @param [in] key         设定项名称
/// @retval 见错误码表
int XPR_UPS_GetInteger(const char* key, int* value);
int XPR_UPS_GetIntegerVK(int* value, const char* key, ...);

/// @brief 设置 64 位整数型设定项值
/// @param [in] token       已打开的设定实例
/// @param [in] key         设定项名称
/// @param [in] value       设定项值
/// @retval 见错误码表
int XPR_UPS_SetInt64(const char* key, int64_t value);
int XPR_UPS_SetInt64VK(int64_t value, const char* key, ...);

/// @brief 获取 64 位整数型设定项值
/// @param [in] token       已打开的设定实例
/// @param [in] key         设定项名称
/// @retval 见错误码表
int XPR_UPS_GetInt64(const char* key, int64_t* value);
int XPR_UPS_GetInt64VK(int64_t* value, const char* key, ...);

/// @brief 设置浮点数型设定项值
/// @param [in] token       已打开的设定实例
/// @param [in] key         设定项名称
/// @param [in] value       设定项值
/// @retval 见错误码表
int XPR_UPS_SetFloat(const char* key, float value);
int XPR_UPS_SetFloatVK(float value, const char* key, ...);

/// @brief 获取浮点数型设定项值
/// @param [in] token       已打开的设定实例
/// @param [in] key         设定项名称
/// @retval 见错误码表
int XPR_UPS_GetFloat(const char* key, float* value);
int XPR_UPS_GetFloatVK(float* value, const char* key, ...);

/// @brief 设置双精度浮点数型设定项值
/// @param [in] token       已打开的设定实例
/// @param [in] key         设定项名称
/// @param [in] value       设定项值
/// @retval 见错误码表
int XPR_UPS_SetDouble(const char* key, double value);
int XPR_UPS_SetDoubleVK(double value, const char* key, ...);

/// @brief 获取双精度浮点数型设定项值
/// @param [in] token       已打开的设定实例
/// @param [in] key         设定项名称
/// @retval 见错误码表
int XPR_UPS_GetDouble(const char* key, double* value);
int XPR_UPS_GetDoubleVK(double* value, const char* key, ...);

/// @brief 设置布尔型设定项值
/// @param [in] token       已打开的设定实例
/// @param [in] key         设定项名称
/// @param [in] value       设定项值
/// @retval 见错误码表
int XPR_UPS_SetBoolean(const char* key, int value);
int XPR_UPS_SetBooleanVK(int value, const char* key, ...);

/// @brief 获取布尔型设定项值
/// @param [in] token       已打开的设定实例
/// @param [in] key         设定项名称
/// @retval 见错误码表
int XPR_UPS_GetBoolean(const char* key, int* value);
int XPR_UPS_GetBooleanVK(int* value, const char* key, ...);

#ifndef XPR_UPS_BLOB_TYPE_DEFINED
#define XPR_UPS_BLOB_TYPE_DEFINED
/// @brief 二进制型设定项数据
struct XPR_UPS_Blob {
    void* data; ///< 数据指针
    int size;   ///< 数据大小
};
typedef struct XPR_UPS_Blob XPR_UPS_Blob;
#endif // XPR_UPS_BLOB_TYPE_DEFINED

/// @brief 设置二进制型设定项值
/// @param [in] token       已打开的设定实例
/// @param [in] key         设定项名称
/// @param [in] value       设定项值
/// @retval 见错误码表
int XPR_UPS_SetBlob(const char* key, XPR_UPS_Blob blob);
int XPR_UPS_SetBlobVK(XPR_UPS_Blob blob, const char* key, ...);

/// @brief 获取二进制型设定项值
/// @param [in] token       已打开的设定实例
/// @param [in] key         设定项名称
/// @retval 见错误码表;
int  XPR_UPS_GetBlob(const char* key, XPR_UPS_Blob* value);
int  XPR_UPS_GetBlobVK(XPR_UPS_Blob* value, const char* key, ...);

/// @brief 删除指定设定项
/// @param [in] key         设定项名称
/// @retval 见错误码表
int XPR_UPS_Delete(const char* key);
int XPR_UPS_DeleteVK(const char* key, ...);

/// @brief 检测设定项是否存在
/// @param [in] key         设定项名称
/// @retval 见错误码表
int XPR_UPS_Exists(const char* key);
int XPR_UPS_ExistsVK(const char* key, ...);

/// @brief 获取设定中的首个设定项名称
/// @retval 见错误码表
const char* XPR_UPS_FirstKey(void);

/// @brief 获取设定中的下一个键名
/// @param [in] key         当前键名
/// @retval 见错误码表
const char* XPR_UPS_NextKey(const char* key);

/// @brief 开始分组操作
/// @param [in] group       分组名称
void XPR_UPS_BeginGroup(const char* group);

/// @brief 结束分组操作
/// @param [in] group       分组名称
void XPR_UPS_EndGroup(const char* group);

/// @brief 导出数据为平坦文件
/// @param [in] url         平坦文件路径
/// @retval 见错误码表
int XPR_UPS_Export(const char* url);

/// @brief 从平坦文件导入数据
/// @param [in] url         平坦文件路径
/// @retval 见错误码表
int XPR_UPS_Import(const char* url);

/// @brief 压实数据文件
/// @retval 见错误码表
int XPR_UPS_Pack(void);

/// @brief 同步数据到存储介质
/// @retval 见错误码表
int XPR_UPS_Sync(void);

#ifdef __cplusplus
}
#endif

/// @}
///

#endif // XPR_UPS_H

