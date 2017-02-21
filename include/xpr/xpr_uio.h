#ifndef XPR_UIO_H
#define XPR_UIO_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

///
/// 合成端口句柄
///
#define XPR_UIO_PORT(major, minor)      (((int)major) << 16 | (int)minor)

///
/// 提取端口主编号
///
#define XPR_UIO_PORT_MAJOR(port)        ((port)>>16 & 0xffff)

///
/// 提取端口子编号
///
#define XPR_UIO_PORT_MINOR(port)        ((port) & 0xffff)

///
/// 目标为所有主/子端口
///
#define XPR_UIO_PORT_ALL                (0xffff)

///
/// 目标为任一主/子端口
///
#define XPR_UIO_PORT_ANY                (0xfffe)

///
/// 目标为空的主/子端口
///
#define XPR_UIO_PORT_NUL                (0x0000)
#define XPR_UIO_WORKER_ALL              (0xffff)
#define XPR_UIO_WORKER_ANY              (0xfffe)

#ifndef XPR_UIO_CONFIGTYPE_TYPE_DEFINED
#define XPR_UIO_CONFIGTYPE_TYPE_DEFINED
///
/// 配置类型枚举定义
///
typedef enum XPR_UIO_ConfigType {
    XPR_UIO_CFG_MAX_PORTS,              ///< 配置最大端口数, (int){16..4096}
    XPR_UIO_CFG_MAX_WORKERS,            ///< 配置最大工作线程数, (int){1..8}
    XPR_UIO_CFG_WORKER_IDLE_THRESH,     ///< 配置工作线程空闲阈值, 单位为 us, (int){100..10000}
    XPR_UIO_CFG_WORKER_IDLE_SLEEP_TIME, ///< 配置工作现场空闲时睡眠时长, 单位为 us, (int){100..100000}
} XPR_UIO_ConfigType;
#endif // XPR_UIO_CONFIGTYPE_TYPE_DEFINED

#ifndef XPR_UIO_ENDPOINTTYPE_TYPE_DEFINED
#define XPR_UIO_ENDPOINTTYPE_TYPE_DEFINED
///
/// 端点类型枚举定义
///
typedef enum XPR_UIO_EndPointType {
    XPR_UIO_ENDPOINT_TYPE_UNKNOWN,      ///< 未知端点
    XPR_UIO_ENDPOINT_TYPE_FILE,         ///< 文件型端点
    XPR_UIO_ENDPOINT_TYPE_INET,         ///< IPv4 网络端点
    XPR_UIO_ENDPOINT_TYPE_INET6,        ///< IPv6 网络端点
    XPR_UIO_ENDPOINT_TYPE_MAX,          ///< 端点类型最大值
} XPR_UIO_EndPointType;
#endif // XPR_UIO_ENDPOINTTYPE_TYPE_DEFINED

#ifndef XPR_UIO_STATE_TYPE_DEFINED
#define XPR_UIO_STATE_TYPE_DEFINED
///
/// 状态类型枚举定义
///
typedef enum XPR_UIO_State {
    XPR_UIO_STATE_UNKNOWN,              ///< 未知状态
    XPR_UIO_STATE_BIND,                 ///< 绑定状态
    XPR_UIO_STATE_CONNECT,              ///< 连接状态
    XPR_UIO_STATE_LISTEN,               ///< 监听状态
} XPR_UIO_State;
#endif // XPR_UIO_STATE_TYPE_DEFINED

#ifndef XPR_UIO_ENDPOINT_TYPE_DEFINED
#define XPR_UIO_ENDPOINT_TYPE_DEFINED
///
/// 端点定义
///
typedef struct XPR_UIO_EndPoint {
    int type;                           ///< 端点类型
    int port;                           ///< 端点端口
    uint8_t data[128];                  ///< 端点数据
} XPR_UIO_EndPoint;
#endif // XPR_UIO_ENDPOINT_TYPE_DEFINED

///
/// 异步接受处理函数
///
typedef int (*XPR_UIO_AsyncAcceptHandler)(void* opaque, int conn,
                                          const XPR_UIOEndPoint* localEP,
                                          const XPR_UIOEndPoint* remoteEP);

///
/// 异步调用例程
///
typedef int (*XPR_UIO_AsyncCallRoutine)(void* opaque, int64_t cts);

///
/// 异步读入数据处理函数
///
typedef int (*XPR_UIO_AsyncReadHandler)(void* oapque, uint8_t* buffer, int length);

///
/// 异步写出数据出来函数
///
typedef int (*XPR_UIO_AsyncWriteHandler)(void* opaque, int status,
                                         int64_t writtenBytes, int64_t totalBytes,
                                         uint8_t** newData, int* newDataLength);

// XPR_UIO 全局接口
//=============================================================================
///
/// 配置 XPR_UIO 核心参数
///
/// @param [in] cfg             配置类型, [XPR_UIO_ConfigType]
/// @param [in] data            配置数据, 数据类型由 cfg 确定
/// @param [in] length          配置数据字节数, 0 表示字面含义,
///                                 > 0 表示数据长度确定的指针,
///                                 < 0 表示 NUL 结尾的字符串
/// @retval XPR_ERR_OK      配置成功
/// @retval XPR_ERR_ERROR   配置失败
/// @note 此接口需要在 XPR_UIO_Init() 之前调用方可生效
/// @code 使用示例
/// {
///     // 配置最大工作线程数为 2
///     XPR_UIO_Config(XPR_UIO_CFG_MAX_WORKERS, (const void*)2, 0);
/// }
/// @endcode
///
int XPR_UIO_Config(int cfg, const void* data, int length);

///
/// 初始化 XPR_UIO 模块
///
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_UIO_Init(void);

///
/// 释放 XPR_UIO 模块已分配的资源
///
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_UIO_Fini(void);

// XPR_UIO 端口相关接口
//==============================================================================
///
/// 打开指定端口
///
/// @param [in] port            端口句柄
/// @param [in] url             URL 格式的目标地址
/// @retval XPR_ERR_OK          成功
/// @retval XPR_ERR_ERROR       失败
/// @retval > 0                 端口句柄
/// @code 使用示例
int XPR_UIO_Open(int port, const char* url);

///
/// 关闭指定端口
///
/// @param [in] port            端口句柄
/// @retval XPR_ERR_OK          成功
/// @retval XPR_ERR_ERROR       失败
int XPR_UIO_Close(int port);

///
/// 检测指定端口状态
///
/// @param [in] port            端口句柄
/// @param [in] state           要检测的状态
/// @retval XPR_TRUE            处于状态
/// @retval XPR_FALSE           不处于状态
int XPR_UIO_CheckState(int port, int state);

int XPR_UIO_GetWorkerId(int port);

int XPR_UIO_SetWorkerId(int port, int workerId);

int64_t XPR_UIO_GetCTS(int port);

int XPR_UIO_SetCTS(int port, int64_t cts);

int XPR_UIO_AsyncAccept(int port, int flags,
                        XPR_UIO_AsyncAcceptHandler cb, void* opaque);

int XPR_UIO_AsyncRead(int port, uint8_t* buffer, int length, int flags,
                      XPR_UIO_AsyncReadHandler cb, void* opaque);

int XPR_UIO_AsyncReadSome(int port, uint8_t* buffer, int size, int flags,
                          XPR_UIO_AsyncReadHandler cb, void* opaque);

int XPR_UIO_AsyncWrite(int port, const uint8_t* data, int length, int flags,
                       XPR_UIO_AsyncWriteHandler cb, void* opaque);

int XPR_UIO_Accept(int port, int* conn, XPR_UIO_EndPoint* localEP,
                   XPR_UIO_EndPoint* remoteEP);

int XPR_UIO_Read(int port, uint8_t* buffer, int length);

int XPR_UIO_ReadSome(int port, uint8_t* buffer, int size);

int XPR_UIO_Write(int port, const uint8_t* data, int length);

// XPR_UIO_EndPoint
//=============================================================================
int XPR_UIO_EndPointIsIPv4(const XPR_UIO_EndPoint* ep);

int XPR_UIO_EndPointIsIPv6(const XPR_UIO_EndPoint* ep);

int XPR_UIO_EndPointIsPrivateNetwork(const XPR_UIO_EndPoint* ep);

int XPR_UIO_EndPointToIPv4String(const XPR_UIO_EndPoint* ep, char* buffer,
                                 int bufferSize);

int XPR_UIO_EndPointToIPv6String(const XPR_UIO_EndPoint* ep, char* buffer,
                                 int bufferSize);

int XPR_UIO_AsyncCall(int workerId, XPR_UIO_AsyncCallRoutine routine, void* opaque);

int XPR_UIO_DelayedAsyncCall(int workerId, XPR_UIO_AsyncCallRoutine routine,
                             void* opaque, int64_t us);

#ifdef __cplusplus
}
#endif

#endif // XPR_UIO_H
