#ifndef XPR_RTSP_H
#define XPR_RTSP_H

#include <stdint.h>
#include "xpr_streamblock.h"

#ifdef __cplusplus
extern "C" {
#endif


//==============================================================================
// PORT 的解析
//
// 本库的所有操作都是基于 PORT 来实现, PORT 的意思是端口, 在本库中代表着目标、通道的含义
// PORT 有 4 部分构成：
//     major   主目标, 用来区分实例化对象类型, 1 =  RTSP 客户端, 2 = RTSP 服务端
//     minor   次目标, 用来指定实例化对象
//     stream  流目标, 用来指定实例化对象的流
//     track   轨道目标, 用来指定实例化对象的流的轨道
//
// 例如：
//     PORT = 0x10010000 = RTSP 客户端实例的第一个目标
//     PORT = 0x20010000 = RTSP 服务端实例的第一个目标, 忽略流及轨道
//     PORT = 0x20010011 = RTSP 服务端实例的第一个目标第一个流的第一个轨道
//
// 每部分都有 3 个值是有特殊含义的:
//     0    表示此部分被忽略
//     M    表示所有目标
//     M-1  表示可以目标中的任一
// * (M) 是每部分的最大值, 例如 major 是 4 个位, 其最大值是 0xF
//
//==============================================================================
 
#define XPR_RTSP_PORT(major, minor, stream, track)      ((((major)<<28))|((minor)<<16)|((stream)<<4)|(track))
#define XPR_RTSP_PORT_MAJOR(port)                       (((port)>>28) & 0x0000000f)
#define XPR_RTSP_PORT_MINOR(port)                       (((port)>>16) & 0x00000fff)
#define XPR_RTSP_PORT_STREAM(port)                      (((port)>>4) & 0x00000fff)
#define XPR_RTSP_PORT_TRACK(port)                       ((port) & 0x0000000f)

#define XPR_RTSP_PORT_MAJOR_ALL                         0x0f
#define XPR_RTSP_PORT_MAJOR_ANY                         0x0e
#define XPR_RTSP_PORT_MAJOR_NUL                         0x00
#define XPR_RTSP_PORT_MAJOR_MIN                         0x01
#define XPR_RTSP_PORT_MAJOR_MAX                         0x0d

#define XPR_RTSP_PORT_MINOR_ALL                         0xfff
#define XPR_RTSP_PORT_MINOR_ANY                         0xffe
#define XPR_RTSP_PORT_MINOR_NUL                         0x000
#define XPR_RTSP_PORT_MINOR_MIN                         0x001
#define XPR_RTSP_PORT_MINOR_MAX                         0xffd

#define XPR_RTSP_PORT_STREAM_MIN                        0x001
#define XPR_RTSP_PORT_STREAM_MAX                        0xffd

#define XPR_RTSP_PORT_TRACK_MIN                         0x01
#define XPR_RTSP_PORT_TRACK_MAX                         0x0d

#define XPR_RTSP_PORT_CLI_ALL                           0x1fff0000
#define XPR_RTSP_PORT_CLI_ANY                           0x1ffe0000

#define XPR_RTSP_PORT_SVR_ALL                           0x2fff0000
#define XPR_RTSP_PORT_SVR_ANY                           0x2ffe0000

#define XPR_RTSP_PORT_MAJOR_CLI                         1
#define XPR_RTSP_PORT_MAJOR_SVR                         2



#ifndef XPR_RTSP_CONFIGTYPE_TYPE_DEFINED
#define XPR_RTSP_CONFIGTYPE_TYPE_DEFINED
///
/// 库参数配置类型
///
typedef enum XPR_RTSP_ConfigType {
    XPR_RTSP_CFG_TICKS,     ///< 运行核心调度器循序间隔
    XPR_RTSP_CFG_BWTPP,     ///< 绑定每个工作线程到特定的CPU上, 数据类型为： int, 有效值为: 0/1
    XPR_RTSP_CFG_WRKPR,     ///< 调整工作线程优先级, 数据类型为: int, 有效值为: -2 ~ +2
    XPR_RTSP_CFG_SRCEV,     ///< 启用或禁用数据源事件通知, 数据类型为: int, 有效值为: 0/1
} XPR_RTSP_ConfigType;
#endif // XPR_RTSP_CONFIGTYPE_TYPE_DEFINED

#ifndef XPR_RTSP_EVENTTYPE_TYPE_DEFINED
#define XPR_RTSP_EVENTTYPE_TYPE_DEFINED
///
/// 事件类型定义
///
typedef enum XPR_RTSP_EventType {
    XPR_RTSP_EVENT_UNKOWN,          ///< 未知事件
    XPR_RTSP_EVENT_BIND_FAILED,     ///< 绑定失败
    XPR_RTSP_EVENT_BYE,             ///< 收到 BYE 消息
    XPR_RTSP_EVENT_CLOSED,          ///< 已经关闭
    XPR_RTSP_EVENT_CLOSING,         ///< 正在关闭
    XPR_RTSP_EVENT_LISTEN_FAILED,   ///< 监听失败
    XPR_RTSP_EVENT_SRC_STARTED,     ///< 数据源已经启动
    XPR_RTSP_EVENT_SRC_STOPPED,     ///< 数据源已经停止
    XPR_RTSP_EVENT_STARTED,         ///< RTSP 对象已经启动
    XPR_RTSP_EVENT_STOPPED,         ///< RTSP 对象已经停止
    XPR_RTSP_EVENT_TIMEOUT,         ///< 数据传输超时
    XPR_RTSP_EVENT_UNREACHABLE,     ///< 传输目标不可达
} XPR_RTSP_EventType;
#endif // XPR_RTSP_EVENTTYPE_TYPE_DEFINED

#ifndef XPR_RTSP_PARAMTYPE_TYPE_DEFINED
#define XPR_RTSP_PARAMTYPE_TYPE_DEFINED
///
/// 参数类型定义
///
typedef enum XPR_RTSP_ParamType {
    XPR_RTSP_PARAM_UNKNOWN,         ///< 未知参数
    XPR_RTSP_PARAM_CACHE_TIME,      ///< 缓冲时长, 单位为秒
    XPR_RTSP_PARAM_FOURCC,          ///< 编码格式
    XPR_RTSP_PARAM_CHANNELS,        ///< 通道数
    XPR_RTSP_PARAM_SAMPLERATE,      ///< 采样率
    XPR_RTSP_PARAM_PPS_DATA,        ///< H264 PPS 数据
    XPR_RTSP_PARAM_SPS_DATA,        ///< H264 SPS 数据
    XPR_RTSP_PARAM_AUTO_SDP,        ///< 自动检测并生成 SDP
    XPR_RTSP_PARAM_JPEG_QFACTOR,    ///< JPEG 编码质量, 有效值为 0 ~ 100
    XPR_RTSP_PARAM_MAX_SESSIONS,    ///< RTSP 服务器对象最大会话数
    XPR_RTSP_PARAM_MAX,
} XPR_RTSP_ParamType;
#endif // XPR_RTSP_PARAMTYPE_TYPE_DEFINED

#ifndef XPR_RTSP_TRANSFERMODE_TYPE_DEFINED
#define XPR_RTSP_TRANSFERMODE_TYPE_DEFINED
///
/// 传输规范类型定义
///
typedef enum XPR_RTSP_TransportSpec {
    XPR_RTSP_TRSPEC_RTP_AVP_AUTO,   ///< RTP/AVP 自动检测
    XPR_RTSP_TRSPEC_RTP_AVP_TCP,    ///< RTP/AVP 基于 TCP 协议传输
    XPR_RTSP_TRSPEC_RTP_AVP_UDP,    ///< RTP/AVP 基于 UDP 协议传输
    XPR_RTSP_TRSPEC_RTP_AVP_MCAST,  ///< RTP/AVP 基于 多播 协议传输
} XPR_RTSP_TransportSpec;
#endif // XPR_RTSP_TRANSFERMODE_TYPE_DEFINED

#ifndef XPR_RTSP_EVENTDATA_TYPE_DEFINED
#define XPR_RTSP_EVENTDATA_TYPE_DEFINED
///
/// 事件数据定义
///
typedef struct XPR_RTSP_EventData {
    int event;      ///< 事件编号, 参见 [XPR_RTSP_EventType]
    void* data;     ///< 事件数据地址 [可选]
    int dataSize;   ///< 事件数据长度 [可选]
} XPR_RTSP_EventData;
#endif // XPR_RTSP_EVENTDATA_TYPE_DEFINED

///
/// 流数据数据回调函数定义
///
/// @param [in] opaque	    用户关联数据
/// @param [in] port        产生数据的端口句柄
/// @param [in] block		数据块信息
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
/// @warning 切勿在此回调函数中调用任何带有 XPR_RTSP_ 前缀的接口
typedef int (*XPR_RTSP_DataCallback)(void* opaque, int port, const XPR_StreamBlock* block);

/// 
/// 流事件通知回调函数
///
/// @param [in] opaque		用户关联数据
/// @param [in] port        产生事件的端口句柄
/// @param [in] evd   		消息事件数据
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
/// @warning 切勿在此回调函数中调用任何带有 XPR_RTSP_ 前缀的接口
typedef int (*XPR_RTSP_EventCallback)(void* opaque, int port, const XPR_RTSP_EventData* evd);

///
/// 配置 XPR_RTSP 初始化前参数
///
/// @param [in] cfg         参数类型, 参见 [#XPR_RTSP_ConfigType]
/// @param [in] data        参数值数据
/// @param [in] length      参数值数据长度
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
/// @note 此接口必须在 XPR_RTSP_Init() 前调用
int XPR_RTSP_Config(int cfg, const char* data, int length);

///
/// 初始化 XPR_RTSP 模块
///
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_RTSP_Init(void);

///
/// 释放 XPR_RTSP 模块已分配的资源
///
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_RTSP_Fini(void);

///
/// 检测端口句柄是否有效
///
/// @param [in] port        端口句柄
/// @retval XPR_TRUE        有效
/// @retval XPR_FALSE       无效
int XPR_RTSP_IsPortValid(int port);

///
/// 打开 RTSP 对象
/// 
/// @param [in] port        端口句柄
/// @param [in] url         RTSP 对象地址, 地址格式依据 RTSP 对象类型而定
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
/// @retval Other           RTSP 对象端口号
/// @code 使用示例
/// {
///     int retval = 0;
///     int port = 0;
///     // 在目标 1 上打开 RTSP 客户端对象
///     port = XPR_RTSP_PORT(XPR_RTSP_PORT_MAJOR_CLI, 1, 0, 0);
///     retval = XPR_RTSP_Open(port, "rtsp://192.168.1.88/live0");
///     if (retval == XPR_ERR_OK)
///         ...
///     ...
///     // 在任一可用目标上打开 RTSP 客户端对象
///     retval = XPR_RTSP_Open(XPR_RTSP_PORT_CLI_ANY, "rtsp://192.168.1.88/live0");
///     if (retval != XPR_ERR_ERROR)
///         port = retval;
///     ...
///     // 在任一可用目标上打开 RTSP 服务端对象
///     retval = XPR_RTSP_Open(XPR_RTSP_PORT_SVR_ANY, "tcp.svr://0.0.0.0:554");
///     if (retval != XPR_ERR_ERROR)
///         port = retval;
///     ...
/// }
/// @endcode
///
int XPR_RTSP_Open(int port, const char* url);

///
/// 关闭已打开的 RTSP 对象
///
/// @param [in] port        端口句柄
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_RTSP_Close(int port);

///
/// 启动 RTSP 对象
///
/// @param [in] port        端口句柄
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_RTSP_Start(int port);

///
/// 停止 RTSP 对象
///
/// @param [in] port        端口句柄
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_RTSP_Stop(int port);

///
/// 允许输出 RTP 数据包
///
/// @param [in] port        端口句柄
/// @param [in] yes         是否启用，有效值: 0 = 否，1 = 是
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_RTSP_EnableRTPPacket(int port, int yes);

///
/// 设置认证信息
///
/// @param [in] port        端口句柄
/// @param [in] username    认证所需的用户名
/// @param [in] password    认证所需的用户密码
/// @param [in] pwdIsMD5    指示 password 是否已经为 MD5 值, 有效值: 0 = 否，1 = 是
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
/// @note 本接口必须在 XPR_RTSP_Start() 之前调用方可生效
int XPR_RTSP_SetAuth(int port, const char* username, const char* password, int pwdIsMD5);
    
///
/// 设置输出数据格式
///
/// @param [in] port        端口句柄
/// @param [in] fourcc      FourCC 格式编码
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_RTSP_SetOutputFormat(int port, int fourcc);

///
/// 设置 RTP 数据超时
///
/// @param [in] port        端口句柄
/// @param [in] us          超时值, 单位为微秒
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_RTSP_SetTimeout(int port, int64_t us);

///
/// 设置 RTP 数据传输规范
///
/// @param [in] port        端口句柄
/// @param [in] trspec      RTP 数据传输规范, 参见 [XPR_RTSP_TransportSpec]
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_RTSP_SetTransportSpec(int port, int trspec);

///
/// 获取指定端口参数
///
/// @param [in] port        端口句柄
/// @param [in,out] buffer  保存参数值的缓冲区
/// @param [in,out] size    保存参数值的缓冲区容量, 返回会实际的数据长度
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_RTSP_GetParam(int port, int param, void* buffer, int* size);

///
/// 设定指定端口参数
///
/// @param [in] port        端口句柄
/// @param [in,out] data    参数值的数据地址
/// @param [in,out] length  参数值的数据长度
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_RTSP_SetParam(int port, int param, const void* data, int length);

///
/// 添加一个新的流
///
/// @param [in] port        端口句柄
/// @param [in] streamName  流的名称
/// @return PORT            流的端口句柄
/// @note 此接口仅用于 RTSP 服务器对象
int XPR_RTSP_NewStream(int port, const char* streamName);

///
/// 销毁一个流
///
/// @param [in] port        流的端口句柄
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_RTSP_DestroyStream(int port);

///
/// 添加一个轨道
///
/// @param [in] port        流的端口句柄
/// @param [in] trackName   轨道的名称
/// @param [in] bandWidth   轨道的最大数据带宽参考值
/// @return PORT            轨道的端口句柄
/// @note 此接口仅用于 RTSP 服务器对象的流对象
int XPR_RTSP_NewTrack(int port, const char* trackName, int bandWidth);

///
/// 销毁一个轨道
///
/// @param [in] port        轨道的端口句柄
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_RTSP_DestroyTrack(int port);

///
/// 通过流的名称查找其关联的端口句柄
///
/// @param [in] port        端口句柄
/// @param [in] streamName  流的名称
/// @return PORT            流的端口句柄
/// @note 此接口仅用于 RTSP 服务器对象
int XPR_RTSP_FindStream(int port, const char* streamName);

///
/// 通过轨道的名称查找其关联的端口句柄
///
/// @param [in] port        端口句柄
/// @param [in] trackName   轨道的名称
/// @return PORT            轨道的端口句柄
/// @note 此接口仅用于 RTSP 服务器对象的流对象
int XPR_RTSP_FindTrack(int port, const char* trackName);

///
/// 获取指定流或轨道的 SDP 数据
///
/// @param [in] port        流或轨道的端口句柄
/// @retval NULL    流或轨道不存在或 SDP 不存在
/// @retval Other   流或轨道的 SDP 数据地址
/// @note 此接口仅用于 RTSP 服务器对象的流或轨道对象
const char* XPR_RTSP_GetSDP(int port);

///
/// 获取指定流或轨道的 SDP 数据长度
///
/// @param [in] port        流或轨道的端口句柄
/// @retval <0      流或轨道不存在
/// @retval =0      流或轨道的 SDP 数据不存在
/// @retval >0      流或轨道的 SDP 数据字节数
/// @note 此接口仅用于 RTSP 服务器对象的流或轨道对象
int XPR_RTSP_GetSDPSize(int port);

///
/// 获取指定流或轨道的名称
///
/// @param [in] port        流或轨道的端口句柄
/// @retval NULL    流或轨道不存在
/// @retval Other   流或轨道的名称
/// @note 此接口仅用于 RTSP 服务器对象的流或轨道对象
const char* XPR_RTSP_GetName(int port);

///
/// 添加数据回调函数
///
/// @param [in] port        端口句柄
/// @param [in] cb          回调函数地址
/// @param [in] opaque      用户关联数据
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_RTSP_AddDataCallback(int port, XPR_RTSP_DataCallback cb, void* opaque);

///
/// 删除数据回调函数
///
/// @param [in] port        端口句柄
/// @param [in] cb          回调函数地址
/// @param [in] opaque      用户关联数据
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_RTSP_DelDataCallback(int port, XPR_RTSP_DataCallback cb, void* opaque);

///
/// 添加事件回调函数
///
/// @param [in] port        端口句柄
/// @param [in] cb          回调函数地址
/// @param [in] opaque      用户关联数据
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_RTSP_AddEventCallback(int port, XPR_RTSP_EventCallback cb, void* opaque);

///
/// 删除事件回调函数
///
/// @param [in] port        端口句柄
/// @param [in] cb          回调函数地址
/// @param [in] opaque      用户关联数据
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_RTSP_DelEventCallback(int port, XPR_RTSP_EventCallback cb, void* opaque);

///
/// 向对象推入数据
///
/// @param [in] port        端口句柄
/// @param [in] block       数据块地址
/// @note 此接口仅用于 RTSP 服务器对象的流的轨道对象
int XPR_RTSP_PushData(int port, XPR_StreamBlock* block);

#ifdef __cplusplus
}
#endif

#endif // XPR_RTSP_H
