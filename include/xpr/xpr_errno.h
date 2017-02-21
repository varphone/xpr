#ifndef XPR_ERRNO_H
#define XPF_ERRNO_H

#include <stdint.h>

/// @defgroup xprerrno 错误代码 
/// @brief     错误代码定义
/// @author    Varphone Wong [varphone@163.com]
/// @version   1.0.0
/// @date      2013/4/1
///
/// @{
///

/// @page xprerrno-changes 变更日志
///
/// @par 1.0.0 (2012/12/20)
///   - 初始版本建立
///

#ifdef __cplusplus
extern "C" {
#endif

#define XPR_ERR_APPID  0x80000000

#ifndef XPR_ERRORLEVEL_TYPE_DEFINED
#define XPR_ERRORLEVEL_TYPE_DEFINED
/// @ingroup xprerrno
/// @brief 错误等级
///
///   定义错误等级，值越大表示错误越致命
///
typedef enum XPR_ErrorLevel {
    XPR_ERR_LEVEL_DEBUG = 0,  ///< debug-level
    XPR_ERR_LEVEL_INFO,       ///< informational
    XPR_ERR_LEVEL_NOTICE,     ///< normal but significant condition
    XPR_ERR_LEVEL_WARNING,    ///< warning conditions
    XPR_ERR_LEVEL_ERROR,      ///< error conditions
    XPR_ERR_LEVEL_CRIT,       ///< critical conditions
    XPR_ERR_LEVEL_ALERT,      ///< action must be taken immediately
    XPR_ERR_LEVEL_FATAL,      ///< just for compatibility with previous version
    XPR_ERR_LEVEL_BUTT        ///< butt
} XPR_ErrorLevel;
#endif // XPR_ERRORLEVEL_TYPE_DEFINED

#ifndef XPR_MODULEID_TYPE_DEFINED
#define XPR_MODULEID_TYPE_DEFINED
/// @ingroup xprerrno
/// @brief 模块编号
///
///   定义每个内置模块的编号
///
typedef enum XPR_ModuleId {
    XPR_MOD_ID_UPS      = 0, ///< UPS
    XPR_MOD_ID_PLUGIN   = 1  ///< PLUGIN
} XPR_ModuleId;
#endif // XPR_MODULEID_TYPE_DEFINED


/// @ingroup xprerrno
/// @brief 定义错误代码
/// @param module       模块编号, see [#XPR_ModuleId]
/// @param level        错误等级, see [#XPR_ErrorLevel]
/// @param errid        错误代码, see [#XPR_ErrorCode]
/// @remark 错误代码格式
///   1    |   APP_ID  |   MOD_ID    | ERR_LEVEL |   ERR_ID   
///   -----|-----------|-------------|-----------|------------
///   1bit |   7bits   |    8bits    |   3bits   |   13bits   
///
#define XPR_DEF_ERR(module, level, errid) \
    ((int32_t)( (XPR_ERR_APPID) | ((module) << 16 ) | ((level)<<13) | (errid) ))

#ifndef XPR_ERRORCODE_TYPE_DEFINED
#define XPR_ERRORCODE_TYPE_DEFINED
/// @ingroup xprerrno
/// @brief 错误代码
///
///   下面定义的为通用错误号，每个模块都有这些错误，具体的可以需要那些就定义那些
///   所有的模块必须预留0~63的错误号，要定义通用错误外的错误请使用63+以上的错误号
///
typedef enum XPR_ErrorCode {
    XPR_ERR_INVALID_DEVID = 1, ///< 无效的设备ID
    XPR_ERR_INVALID_CHNID = 2, ///< 无效的通道 ID
    XPR_ERR_ILLEGAL_PARAM = 3, ///< 至少一个参数是无效的, eg: 例如无效的IP地址
    XPR_ERR_EXIST         = 4, ///< 资源已经存在, eg: 挂在相同的目录
    XPR_ERR_UNEXIST       = 5, ///< 资源不存在, eg: 目录没有挂在就挂在节点
    XPR_ERR_NULL_PTR      = 6, ///< 传入的指针为空

    XPR_ERR_NOT_CONFIG    = 7, ///< 某些操作需要事先配置

    XPR_ERR_NOT_SUPPORT   = 8, ///< 现在海不支持当前操作
    XPR_ERR_NOT_PERM      = 9, ///< 操作不允许修改, eg: 修改静态属性

    XPR_ERR_NOMEM         = 12, ///< 申请内存失败 malloc
    XPR_ERR_NOBUF         = 13, ///< failure caused by malloc buffer

    XPR_ERR_BUF_EMPTY     = 14, ///< 缓冲区为空
    XPR_ERR_BUF_FULL      = 15, ///< 缓冲区已满

    XPR_ERR_SYS_NOTREADY  = 16, ///< 系统未初始化 ，需要先调用Init或Load

    XPR_ERR_BADADDR       = 17, ///< 错误的地址, eg: used for copy_from_user & copy_to_user
    XPR_ERR_BUSY          = 18, ///< 资源正忙

    XPR_ERR_DUMP          = 19, ///< 写入文件失败
    XPR_ERR_WRITE         = 20, ///< 保存失败

    XPR_ERR_BUTT          = 63, ///< 保留错误好的最大码，所有模块必须大于此

} XPR_ErrorCode;
#endif // XPR_ERRORCODE_TYPE_DEFINED

// Generic
//==============================================================================
///
/// 发生错误
///
#define XPR_ERR_ERROR       0xffffffff

///
/// 没有异常
///
#define XPR_ERR_OK          0x00000000

///
/// 执行成功
///
#define XPR_ERR_SUCCESS     0x00000000

// UPS
//==============================================================================
///
/// 传进去的参数无效
///
#define XPR_ERR_UPS_ILLEGAL_PARAM    XPR_DEF_ERR(XPR_MOD_ID_UPS, XPR_ERR_LEVEL_ERROR, XPR_ERR_ILLEGAL_PARAM)

///
/// 资源已存在
///
#define XPR_ERR_UPS_EXISTED          XPR_DEF_ERR(XPR_MOD_ID_UPS, XPR_ERR_LEVEL_ERROR, XPR_ERR_EXIST)

///
/// 资源不存在
///
#define XPR_ERR_UPS_UNEXIST          XPR_DEF_ERR(XPR_MOD_ID_UPS, XPR_ERR_LEVEL_ERROR, XPR_ERR_UNEXIST)

///
/// 指针为空
///
#define XPR_ERR_UPS_NULL_PTR         XPR_DEF_ERR(XPR_MOD_ID_UPS, XPR_ERR_LEVEL_ERROR, XPR_ERR_NULL_PTR)

///
/// 系统未配置
///
#define XPR_ERR_UPS_NOT_CONFIG       XPR_DEF_ERR(XPR_MOD_ID_UPS, XPR_ERR_LEVEL_ERROR, XPR_ERR_NOT_CONFIG)

///
/// 当前操作不支持
///
#define XPR_ERR_UPS_NOT_SUPPORT      XPR_DEF_ERR(XPR_MOD_ID_UPS, XPR_ERR_LEVEL_ERROR, XPR_ERR_NOT_SUPPORT)

///
/// 不允许操作
///
#define XPR_ERR_UPS_NOT_PERM         XPR_DEF_ERR(XPR_MOD_ID_UPS, XPR_ERR_LEVEL_ERROR,  XPR_ERR_NOT_PERM)

///
/// 内薰申请失败
///
#define XPR_ERR_UPS_NOMEM            XPR_DEF_ERR(XPR_MOD_ID_UPS, XPR_ERR_LEVEL_ERROR, XPR_ERR_NOMEM)

///
/// 缓冲区太小
///
#define XPR_ERR_UPS_NOBUF            XPR_DEF_ERR(XPR_MOD_ID_UPS, XPR_ERR_LEVEL_ERROR,  XPR_ERR_NOBUF)

///
/// UPS 未初始化
///
#define XPR_ERR_UPS_NOTREADY         XPR_DEF_ERR(XPR_MOD_ID_UPS, XPR_ERR_LEVEL_ERROR,  XPR_ERR_SYS_NOTREADY)

///
/// UPS 写入文件失败
///
#define XPR_ERR_UPS_DUMP         XPR_DEF_ERR(XPR_MOD_ID_UPS, XPR_ERR_LEVEL_ERROR,  XPR_ERR_DUMP)

///
/// UPS 保存失败
///
#define XPR_ERR_UPS_WRITE         XPR_DEF_ERR(XPR_MOD_ID_UPS, XPR_ERR_LEVEL_ERROR, XPR_ERR_WRITE)

#ifdef __cplusplus
}
#endif

/// @}
///

#endif //XPR_ERRNO_H
