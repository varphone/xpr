/*
 * File: xpr_rtsp.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * RTSP 服务器及客户端接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2016-11-25 11:20:21 Friday, 25 November
 * Last Modified : 2019-07-03 04:44:54 Wednesday, 3 July
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
#ifndef XPR_RTSP_H
#define XPR_RTSP_H

#include <stdint.h>
#include <xpr/xpr_common.h>
#include <xpr/xpr_streamblock.h>

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// PORT 的解析
//
// 本库的所有操作都是基于 PORT 来实现, PORT 的意思是端口, 在本库中代表着目标、通道的含义
// PORT 有 4 部分构成：
//     major            主目标, 用来区分实例化对象类型, 1 =  RTSP 客户端, 2 = RTSP 服务端
//     minor(stream)    次目标, 用来指定实例化对象(流)
//     track            轨道目标, 用来指定实例化对象的流的轨道
//
// 例如：
//     PORT = 0x01000100 = RTSP 客户端实例的第一个目标
//     PORT = 0x02000100 = RTSP 服务端实例的第一个目标, 忽略流及轨道
//     PORT = 0x02000101 = RTSP 服务端实例的第一个目标第一个流的第一个轨道
//
// 每部分都有 3 个值是有特殊含义的:
//     0    表示此部分被忽略
//     M    表示所有目标
//     M-1  表示可以目标中的任一
// * (M) 是每部分的最大值, 例如 major 是 7 个位, 其最大值是 0x7F
//
//==============================================================================
 
#define XPR_RTSP_PORT(major, minor, track)              ((((major)<<24))|((minor)<<8)|(track))
#define XPR_RTSP_PORT_MAJOR(port)                       (((port)>>24) & 0x0000007f)
#define XPR_RTSP_PORT_MINOR(port)                       (((port)>>8) & 0x0000ffff)
#define XPR_RTSP_PORT_STREAM                            XPR_RTSP_PORT_MINOR
#define XPR_RTSP_PORT_TRACK(port)                       ((port) & 0x000000ff)

#define XPR_RTSP_PORT_MAJOR_ALL                         0x7f
#define XPR_RTSP_PORT_MAJOR_ANY                         0x7e
#define XPR_RTSP_PORT_MAJOR_NUL                         0x00
#define XPR_RTSP_PORT_MAJOR_MIN                         0x01
#define XPR_RTSP_PORT_MAJOR_MAX                         0x7d

#define XPR_RTSP_PORT_MINOR_ALL                         0xffff
#define XPR_RTSP_PORT_MINOR_ANY                         0xfffe
#define XPR_RTSP_PORT_MINOR_NUL                         0x0000
#define XPR_RTSP_PORT_MINOR_MIN                         0x0001
#define XPR_RTSP_PORT_MINOR_MAX                         0xfffd

#define XPR_RTSP_PORT_STREAM_ALL                        XPR_RTSP_PORT_MINOR_ALL
#define XPR_RTSP_PORT_STREAM_ANY                        XPR_RTSP_PORT_MINOR_ANY
#define XPR_RTSP_PORT_STREAM_NUL                        XPR_RTSP_PORT_MINOR_NUL
#define XPR_RTSP_PORT_STREAM_MIN                        XPR_RTSP_PORT_MINOR_MIN
#define XPR_RTSP_PORT_STREAM_MAX                        XPR_RTSP_PORT_MINOR_MAX

#define XPR_RTSP_PORT_TRACK_ALL                         0xff
#define XPR_RTSP_PORT_TRACK_ANY                         0xfe
#define XPR_RTSP_PORT_TRACK_NUL                         0x00
#define XPR_RTSP_PORT_TRACK_MIN                         0x01
#define XPR_RTSP_PORT_TRACK_MAX                         0xfd

#define XPR_RTSP_PORT_CLI_ALL                           0x01ffff00
#define XPR_RTSP_PORT_CLI_ANY                           0x01fffe00

#define XPR_RTSP_PORT_SVR_ALL                           0x02ffff00
#define XPR_RTSP_PORT_SVR_ANY                           0x02fffe00

#define XPR_RTSP_PORT_MAJOR_CLI                         1
#define XPR_RTSP_PORT_MAJOR_SVR                         2


#ifndef XPR_RTSP_CFG_TYPE_DEFINED
#define XPR_RTSP_CFG_TYPE_DEFINED
///
/// 库参数配置类型
///
typedef enum XPR_RTSP_CFG {
    XPR_RTSP_CFG_TICKS,     ///< 运行核心调度器循序间隔
    XPR_RTSP_CFG_BWTPP,     ///< 绑定每个工作线程到特定的CPU上, 数据类型为： int, 有效值为: 0/1
    XPR_RTSP_CFG_WRKPR,     ///< 调整工作线程优先级, 数据类型为: int, 有效值为: -2 ~ +2
    XPR_RTSP_CFG_SRCEV,     ///< 启用或禁用数据源事件通知, 数据类型为: int, 有效值为: 0/1
} XPR_RTSP_CFG;
#endif // XPR_RTSP_CFG_TYPE_DEFINED

#ifndef XPR_RTSP_EVT_TYPE_DEFINED
#define XPR_RTSP_EVT_TYPE_DEFINED
///
/// 事件类型定义
///
typedef enum XPR_RTSP_EVT {
    XPR_RTSP_EVT_UNKOWN,          ///< 未知事件
    XPR_RTSP_EVT_BIND_FAILED,     ///< 绑定失败
    XPR_RTSP_EVT_BYE,             ///< 收到 BYE 消息
    XPR_RTSP_EVT_CLOSED,          ///< 已经关闭
    XPR_RTSP_EVT_CLOSING,         ///< 正在关闭
    XPR_RTSP_EVT_LISTEN_FAILED,   ///< 监听失败
    XPR_RTSP_EVT_SRC_STARTED,     ///< 数据源已经启动
    XPR_RTSP_EVT_SRC_STOPPED,     ///< 数据源已经停止
    XPR_RTSP_EVT_STARTED,         ///< RTSP 对象已经启动
    XPR_RTSP_EVT_STOPPED,         ///< RTSP 对象已经停止
    XPR_RTSP_EVT_TIMEOUT,         ///< 数据传输超时
    XPR_RTSP_EVT_UNREACHABLE,     ///< 传输目标不可达
    XPR_RTSP_EVT_UAC,             ///< 用户访问检查
} XPR_RTSP_EVT;
#endif // XPR_RTSP_EVT_TYPE_DEFINED

typedef enum XPR_RTSP_FLAG {
    XPR_RTSP_FLAG_NULL = 0x00,
    XPR_RTSP_FLAG_OPEN = 0x01,
    XPR_RTSP_FLAG_CLOSE = 0x02,
    XPR_RTSP_FLAG_START = 0x04,
    XPR_RTSP_FLAG_STOP = 0x08,
    XPR_RTSP_FLAG_EVTMOPD = 0x10,
    XPR_RTSP_FLAG_EVSSTPD = 0x20,
    XPR_RTSP_FLAG_HKMIPD = 0x40,
    XPR_RTSP_FLAG_KPL = 0x80,
} XPR_RTSP_FLAG;

#ifndef XPR_RTSP_PARAM_TYPE_DEFINED
#define XPR_RTSP_PARAM_TYPE_DEFINED
///
/// 参数类型定义
///
typedef enum XPR_RTSP_PARAM {
    XPR_RTSP_PARAM_UNKNOWN,      ///< 未知参数
    XPR_RTSP_PARAM_CACHE_TIME,   ///< 缓冲时长, 单位为秒
    XPR_RTSP_PARAM_FOURCC,       ///< 编码格式
    XPR_RTSP_PARAM_CHANNELS,     ///< 通道数
    XPR_RTSP_PARAM_SAMPLERATE,   ///< 采样率
    XPR_RTSP_PARAM_PPS_DATA,     ///< H264 PPS 数据
    XPR_RTSP_PARAM_SPS_DATA,     ///< H264 SPS 数据
    XPR_RTSP_PARAM_AUTO_SDP,     ///< 自动检测并生成 SDP
    XPR_RTSP_PARAM_JPEG_QFACTOR, ///< JPEG 编码质量, 有效值为 0 ~ 100
    XPR_RTSP_PARAM_MAX_SESSIONS, ///< RTSP 服务器对象最大会话数
    XPR_RTSP_PARAM_H264_ADD_AUD, ///< 添加 H264 AUD 单元
    XPR_RTSP_PARAM_H264_SINGLE_FRAME, ///< 是否合为单帧输出
    XPR_RTSP_PARAM_H264_STARTCODE,    ///< 是否包含起始码
    XPR_RTSP_PARAM_H264_SA_SPS_PPS,   ///< 是否独立输出 SPS 及 PPS
    XPR_RTSP_PARAM_MAX,
} XPR_RTSP_PARAM;
#endif // XPR_RTSP_PARAM_TYPE_DEFINED

#ifndef XPR_RTSP_TRSPEC_TYPE_DEFINED
#define XPR_RTSP_TRSPEC_TYPE_DEFINED
///
/// 传输规范类型定义
///
typedef enum XPR_RTSP_TRSPEC {
    XPR_RTSP_TRSPEC_RTP_AVP_AUTO,   ///< RTP/AVP 自动检测
    XPR_RTSP_TRSPEC_RTP_AVP_TCP,    ///< RTP/AVP 基于 TCP 协议传输
    XPR_RTSP_TRSPEC_RTP_AVP_UDP,    ///< RTP/AVP 基于 UDP 协议传输
    XPR_RTSP_TRSPEC_RTP_AVP_MCAST,  ///< RTP/AVP 基于 多播 协议传输
} XPR_RTSP_TRSPEC;
#endif // XPR_RTSP_TRSPEC_TYPE_DEFINED

#ifndef XPR_RTSP_EVD_TYPE_DEFINED
#define XPR_RTSP_EVD_TYPE_DEFINED
///
/// 事件数据定义
///
typedef struct XPR_RTSP_EVD {
    XPR_RTSP_EVT event; ///< 事件编号, 参见 [XPR_RTSP_EVT]
    void* data;         ///< 事件数据地址 [可选]
    int data_size;      ///< 事件数据长度 [可选]
} XPR_RTSP_EVD;
#endif // XPR_RTSP_EVD_TYPE_DEFINED

#ifndef XPR_RTSP_EVD_UAC_TYPE_DEFINED
#define XPR_RTSP_EVD_UAC_TYPE_DEFINED
///
/// 用户访问检查事件数据定义
///
typedef struct XPR_RTSP_EVD_UAC {
    int client_socket;                            ///< 客户端套接字
    struct sockaddr_storage const* client_addr;   ///< 客户端地址
    char const* url_suffix;                       ///< 客户端请求路径
    char const* username;                         ///< 客户端请求用户名
} XPR_RTSP_EVD_UAC;
#endif // XPR_RTSP_EVD_UAC_TYPE_DEFINED

///
/// 流数据数据回调函数定义
///
/// @param [in] opaque      用户关联数据
/// @param [in] port        产生数据的端口句柄
/// @param [in] block       数据块信息
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
/// @warning 切勿在此回调函数中调用任何带有 XPR_RTSP_ 前缀的接口
typedef int (*XPR_RTSP_DCB)(void* opaque, int port, const XPR_StreamBlock* block);

/// 
/// 流事件通知回调函数
///
/// @param [in] opaque      用户关联数据
/// @param [in] port        产生事件的端口句柄
/// @param [in] evd         消息事件数据
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
/// @warning 切勿在此回调函数中调用任何带有 XPR_RTSP_ 前缀的接口
typedef int (*XPR_RTSP_EVCB)(void* opaque, int port, const XPR_RTSP_EVD* evd);

///
/// 配置 XPR_RTSP 初始化前参数
///
/// @param [in] cfg         参数类型, 参见 [#XPR_RTSP_CFG]
/// @param [in] data        参数值数据
/// @param [in] length      参数值数据长度
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
/// @note 此接口必须在 XPR_RTSP_Init() 前调用
XPR_API int XPR_RTSP_Config(XPR_RTSP_CFG cfg, const char* data, int length);

///
/// 初始化 XPR_RTSP 模块
///
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
XPR_API int XPR_RTSP_Init(void);

///
/// 释放 XPR_RTSP 模块已分配的资源
///
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
XPR_API int XPR_RTSP_Fini(void);

///
/// 检测端口句柄是否有效
///
/// @param [in] port        端口句柄
/// @retval XPR_TRUE        有效
/// @retval XPR_FALSE       无效
XPR_API int XPR_RTSP_IsPortValid(int port);

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
XPR_API int XPR_RTSP_Open(int port, const char* url);

///
/// 关闭已打开的 RTSP 对象
///
/// @param [in] port        端口句柄
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
XPR_API int XPR_RTSP_Close(int port);

///
/// 启动 RTSP 对象
///
/// @param [in] port        端口句柄
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
XPR_API int XPR_RTSP_Start(int port);

///
/// 停止 RTSP 对象
///
/// @param [in] port        端口句柄
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
XPR_API int XPR_RTSP_Stop(int port);

///
/// 允许输出 RTP 数据包
///
/// @param [in] port        端口句柄
/// @param [in] yes         是否启用，有效值: 0 = 否，1 = 是
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
XPR_API int XPR_RTSP_EnableRTPPacket(int port, int yes);

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
XPR_API int XPR_RTSP_SetAuth(int port, const char* username,
                             const char* password, int pwdIsMD5);

///
/// 设置输出数据格式
///
/// @param [in] port        端口句柄
/// @param [in] fourcc      FourCC 格式编码
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
XPR_API int XPR_RTSP_SetOutputFormat(int port, int fourcc);

///
/// 设置 RTP 数据超时
///
/// @param [in] port        端口句柄
/// @param [in] us          超时值, 单位为微秒
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
XPR_API int XPR_RTSP_SetTimeout(int port, int64_t us);

///
/// 设置 RTP 数据传输规范
///
/// @param [in] port        端口句柄
/// @param [in] trspec      RTP 数据传输规范, 参见 [XPR_RTSP_TRSPEC]
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
XPR_API int XPR_RTSP_SetTrSpec(int port, XPR_RTSP_TRSPEC trspec);

///
/// 获取指定端口参数
///
/// @param [in] port        端口句柄
/// @param [in,out] buffer  保存参数值的缓冲区
/// @param [in,out] size    保存参数值的缓冲区容量, 返回会实际的数据长度
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
XPR_API int XPR_RTSP_GetParam(int port, XPR_RTSP_PARAM param, void* buffer,
                              int* size);

///
/// 设定指定端口参数
///
/// @param [in] port        端口句柄
/// @param [in,out] data    参数值的数据地址
/// @param [in,out] length  参数值的数据长度
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
XPR_API int XPR_RTSP_SetParam(int port, XPR_RTSP_PARAM param, const void* data,
                              int length);

///
/// 添加一个新的流
///
/// @param [in] port        端口句柄
/// @param [in] streamName  流的名称
/// @return PORT            流的端口句柄
/// @note 此接口仅用于 RTSP 服务器对象
XPR_API int XPR_RTSP_NewStream(int port, const char* streamName);

///
/// 销毁一个流
///
/// @param [in] port        流的端口句柄
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
XPR_API int XPR_RTSP_DestroyStream(int port);

///
/// 添加一个轨道
///
/// @param [in] port        流的端口句柄
/// @param [in] trackName   轨道的名称
/// @param [in] bandWidth   轨道的最大数据带宽参考值
/// @return PORT            轨道的端口句柄
/// @note 此接口仅用于 RTSP 服务器对象的流对象
XPR_API int XPR_RTSP_NewTrack(int port, const char* trackName, int bandWidth);

///
/// 销毁一个轨道
///
/// @param [in] port        轨道的端口句柄
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
XPR_API int XPR_RTSP_DestroyTrack(int port);

///
/// 通过流的名称查找其关联的端口句柄
///
/// @param [in] port        端口句柄
/// @param [in] streamName  流的名称
/// @return PORT            流的端口句柄
/// @note 此接口仅用于 RTSP 服务器对象
XPR_API int XPR_RTSP_FindStream(int port, const char* streamName);

///
/// 通过轨道的名称查找其关联的端口句柄
///
/// @param [in] port        端口句柄
/// @param [in] trackName   轨道的名称
/// @return PORT            轨道的端口句柄
/// @note 此接口仅用于 RTSP 服务器对象的流对象
XPR_API int XPR_RTSP_FindTrack(int port, const char* trackName);

///
/// 获取指定流或轨道的 SDP 数据
///
/// @param [in] port        流或轨道的端口句柄
/// @retval NULL    流或轨道不存在或 SDP 不存在
/// @retval Other   流或轨道的 SDP 数据地址
/// @note 此接口仅用于 RTSP 服务器对象的流或轨道对象
XPR_API const char* XPR_RTSP_GetSDP(int port);

///
/// 获取指定流或轨道的 SDP 数据长度
///
/// @param [in] port        流或轨道的端口句柄
/// @retval <0      流或轨道不存在
/// @retval =0      流或轨道的 SDP 数据不存在
/// @retval >0      流或轨道的 SDP 数据字节数
/// @note 此接口仅用于 RTSP 服务器对象的流或轨道对象
XPR_API int XPR_RTSP_GetSDPSize(int port);

///
/// 获取指定流或轨道的名称
///
/// @param [in] port        流或轨道的端口句柄
/// @retval NULL    流或轨道不存在
/// @retval Other   流或轨道的名称
/// @note 此接口仅用于 RTSP 服务器对象的流或轨道对象
XPR_API const char* XPR_RTSP_GetName(int port);

///
/// 添加数据回调函数
///
/// @param [in] port        端口句柄
/// @param [in] cb          回调函数地址
/// @param [in] opaque      用户关联数据
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
XPR_API int XPR_RTSP_AddDataCallback(int port, XPR_RTSP_DCB cb, void* opaque);

///
/// 删除数据回调函数
///
/// @param [in] port        端口句柄
/// @param [in] cb          回调函数地址
/// @param [in] opaque      用户关联数据
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
XPR_API int XPR_RTSP_DelDataCallback(int port, XPR_RTSP_DCB cb, void* opaque);

///
/// 添加事件回调函数
///
/// @param [in] port        端口句柄
/// @param [in] cb          回调函数地址
/// @param [in] opaque      用户关联数据
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
XPR_API int XPR_RTSP_AddEventCallback(int port, XPR_RTSP_EVCB cb, void* opaque);

///
/// 删除事件回调函数
///
/// @param [in] port        端口句柄
/// @param [in] cb          回调函数地址
/// @param [in] opaque      用户关联数据
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
XPR_API int XPR_RTSP_DelEventCallback(int port, XPR_RTSP_EVCB cb, void* opaque);

///
/// 向对象推入数据
///
/// @param [in] port        端口句柄
/// @param [in] block       数据块地址
/// @note 此接口仅用于 RTSP 服务器对象的流的轨道对象
XPR_API int XPR_RTSP_PushData(int port, XPR_StreamBlock* block);

XPR_API int XPR_RTSP_PostData(int port, XPR_StreamBlock* block);

XPR_API int XPR_RTSP_PostEvent(int port, const XPR_RTSP_EVD* evd);

#ifdef __cplusplus
}
#endif

#endif // XPR_RTSP_H
