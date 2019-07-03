/*
 * File: xpr_mcvr.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 多通道视频渲染接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2016-11-25 11:25:25 Friday, 25 November
 * Last Modified : 2019-07-03 05:01:04 Wednesday, 3 July
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
#ifndef XPR_MCVR_H
#define XPR_MCVR_H

#include <stddef.h>
#include <xpr/xpr_avframe.h>
#include <xpr/xpr_common.h>
#include <xpr/xpr_streamblock.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XPR_MCVR_PORT(major, minor) (((major)<<16)|(minor))
#define XPR_MCVR_PORT_MAJOR(port)   (((port)>>16) & 0xffff)
#define XPR_MCVR_PORT_MINOR(port)   ((port) & 0xffff)

/// 错误码
typedef enum XPR_MCVR_ErrorCode {
    XPR_MCVR_ERR_NONE                   = 0,   ///< 无错误
    XPR_MCVR_ERR_UNKNOWN                = 1,   ///< 未知错误
    XPR_MCVR_ERR_RENDERER_CREATE_FAILED = 2,   ///< 渲染器类创建失败
    XPR_MCVR_ERR_NULL_RENDERER          = 3,   ///< 渲染器类为空
    XPR_MCVR_ERR_DEVICE_CREATE_FAILED   = 4,   ///< 设备创建失败
    XPR_MCVR_ERR_VERTEX_CREATE_FAILED   = 5,   ///< 顶点创建失败
    XPR_MCVR_ERR_EFFECT_CREATE_FAILED   = 6,   ///< 效果创建失败
    XPR_MCVR_ERR_THREAD_CREATE_FAILED   = 7,   ///< 线程创建失败
    XPR_MCVR_ERR_TEXTURE_CREATE_FAILED  = 8,   ///< 纹理创建失败
    XPR_MCVR_ERR_IMAGE_ENHANCE_FAILED   = 9,   ///< 图像增强失败
    XPR_MCVR_ERR_NULL_DEVICE            = 10,  ///< 设备为空
    XPR_MCVR_ERR_NULL_CHANNEL           = 11,  ///< 通道为空
    XPR_MCVR_ERR_NULL_TEXTURE           = 12,  ///< 纹理为空
    XPR_MCVR_ERR_INVALID_PORT           = 13,  ///< 通道号无效
    XPR_MCVR_ERR_INVALID_LAYOUT         = 14,  ///< 布局无效
    XPR_MCVR_ERR_INVALID_FACTOR         = 15,  ///< 图像系数无效
    XPR_MCVR_ERR_INVALID_PIXEL_FORMAT   = 16,  ///< 像素格式无效
    XPR_MCVR_ERR_INVALID_INPUT_DATA     = 17,  ///< 无效输入数据
    XPR_MCVR_ERR_NULL_POINTER           = 18,  ///<  无效指针
} XPR_MCVR_ErrorCode;

///
/// 视频渲染器类型
///
typedef enum XPR_MCVR_VideoRendererType {
    XPR_MCVR_VIDEO_RENDERER_NULL    = 0,   ///< 无渲染
    XPR_MCVR_VIDEO_RENDERER_DEFAULT = 1,   ///< 库内默认
    XPR_MCVR_VIDEO_RENDERER_D3D     = 2,   ///< Direct3D
    XPR_MCVR_VIDEO_RENDERER_OPENGL  = 3,   ///< OpenGL
} XPR_MCVR_VideoRendererType;

///
/// 渲染级别
///
typedef enum XPR_MCVR_RenderLevel {
    XPR_MCVR_RENDER_LEVEL_0 = 0,   ///< 执行所有绘制操作
    XPR_MCVR_RENDER_LEVEL_1 = 1,   ///< 不绘制无数据主通道
    XPR_MCVR_RENDER_LEVEL_2 = 2,   ///< 不绘制子通道摄像机状态
    XPR_MCVR_RENDER_LEVEL_3 = 3,   ///< 不绘制标题文本
    XPR_MCVR_RENDER_LEVEL_4 = 4,   ///< 不绘制边框线条
} XPR_MCVR_RenderLevel;

/// 子通道状态
typedef enum XPR_MCVR_ChannelState {
    XPR_MCVR_CHANNEL_NO_CAMERA = 0,    ///< 无设备
    XPR_MCVR_CHANNEL_BUFFERING = 1,    ///< 正在缓冲
    XPR_MCVR_CHANNEL_RENDERING = 2,    ///< 正在渲染
    XPR_MCVR_CHANNEL_INTERRUPT = 3,    ///< 断开连接
    XPR_MCVR_CHANNEL_STOPPED   = 4,    ///< 停止播放
} XPR_MCVR_ChannelState;

/// 事件类型
typedef enum XPR_MCVR_EventType {
    XPR_MCVR_LEFT_CLICK   = 0,    ///< 左键单击事件
    XPR_MCVR_RIGHT_CLICK  = 1,    ///< 右键单击事件
    XPR_MCVR_LEFT_DBCLICK = 2,    ///< 左键双击事件
} XPR_MCVR_EventType;
    
///
/// 字符串显示状态
///
typedef enum XPR_MCVR_StringState {
    XPR_MCVR_STRING_STATE_HIDE   = -1,     ///< 隐藏（与显示相对）
    XPR_MCVR_STRING_STATE_SHOW   =  0,     ///< 显示（与隐藏相对）
    XPR_MCVR_STRING_STATE_SHOW_1 =  1,     ///< 显示字符串表第一项
    XPR_MCVR_STRING_STATE_SHOW_2 =  2,     ///< 显示字符串表第二项
    XPR_MCVR_STRING_STATE_SHOW_3 =  3,     ///< 显示字符串表第三项
    XPR_MCVR_STRING_STATE_SHOW_4 =  4,     ///< 显示字符串表第四项
    XPR_MCVR_STRING_STATE_MAX    =  5,     ///< 预留
} XPR_MCVR_StringState;

///
/// 字符串类型
///
typedef enum XPR_MCVR_StringType {
    XPR_MCVR_STRING_TYPE_TITLE = 1,    ///< 标题栏
    XPR_MCVR_STRING_TYPE_HINT  = 2,    ///< 状态提示
    XPR_MCVR_STRING_TYPE_MAX   = 3,    ///< 预留
} XPR_MCVR_StringType;
        
///
/// 视频效果
///
typedef enum XPR_MCVR_EffectType {
    XPR_MCVR_EFFECT_ALL,                     ///< 所有效果, 有效值：0/1, 0 = 关闭, 1 = 关闭

    XPR_MCVR_EFFECT_ENHANCEMENTS = 0x1000,   ///< ***所有增强类效果, 参数：0/1***
    XPR_MCVR_EFFECT_BRIGHTEN,                ///< 亮度提升, 有效值：0~100, 0 为关闭
    XPR_MCVR_EFFECT_SHARPEN,                 ///< 锐度提升, 有效值: 0~100, 0 为关闭
    XPR_MCVR_EFFECT_HIST_EQUAL,              ///< 直方图均衡, 有效值: 0~100, 0 为关闭
    XPR_MCVR_EFFECT_HIST_STRETCH,            ///< 直方图拉伸, 有效值：0~100, 0 为关闭
        
    XPR_MCVR_EFFECT_PROPERTIES   = 0x2000,   ///< ***所有图像属性类效果, 有效值：0/1, 0 = 关闭, 1 = 开启***
    XPR_MCVR_EFFECT_LUMINANCE,               ///< 亮度调节, 有效值: 0~100, 50为默认值
    XPR_MCVR_EFFECT_CONTRAST,                ///< 对比度调节, 有效值: 0~100, 50为默认值
    XPR_MCVR_EFFECT_GAMMA,                   ///< 伽马调节, 有效值: 0~100, 50为默认值
    XPR_MCVR_EFFECT_HUE,                     ///< 色调调节, 有效值: 0~100, 50为默认值
    XPR_MCVR_EFFECT_SATURATION,              ///< 饱和度调节, 有效值: 0~100, 50为默认值
        
    XPR_MCVR_EFFECT_SPECIALS     = 0x3000,   ///< ***所有特殊效果, 有效值：0/1, 0 = 关闭, 1 = 开启***
    XPR_MCVR_EFFECT_ALARM,                   ///< 报警效果, 有效值：0/1, 0 = 关闭, 1 = 开启
} XPR_MCVR_EffectType;

///
/// 视频效果值类型
///
typedef enum XPR_MCVR_EffectValueType {
    XPR_MCVR_EFFECT_VT_UNKNOWN = 0,      ///< 未知类型
    XPR_MCVR_EFFECT_VT_BOOLEAN = 1,      ///< 布尔
    XPR_MCVR_EFFECT_VT_INTEGER = 2,      ///< 整数
    XPR_MCVR_EFFECT_VT_REAL    = 3,      ///< 实数
    XPR_MCVR_EFFECT_VT_STRING  = 4,      ///< 字符串
} XPR_MCVR_EffectValueType;

///
/// 优化操作码
///
typedef enum XPR_MCVR_OptimizeLevel {
    XPR_MCVR_OPTIMIZE_NONE = 0,
	XPR_MCVR_OPTIMIZE_IMPROVING = 1,
	XPR_MCVR_OPTIMIZE_REDUCING = 2,
} XPR_MCVR_OptimizeLevel;

/// @brief 初始化渲染器实例，每个播放库对应一个渲染器实例
/// @param [in] hwnd 主窗口句柄
/// @param [in] type 渲染器类型，参加 #XPR_MCVR_RendererType
/// @retval 1 成功
/// @retval 0 失败
XPR_API int XPR_MCVR_Init(void* hwnd, XPR_MCVR_VideoRendererType type);

/// @brief 释放渲染器实例已分配资源
/// @return 无返回值
XPR_API int XPR_MCVR_Fini(void);

/// @brief  获得当前视频渲染器类型
/// @return 当前视频渲染器类型
XPR_API XPR_MCVR_VideoRendererType XPR_MCVR_GetVideoRendererType(void);

/// @brief 绑定窗口与主通道号
/// @param [in] port    主通道号 0x0001~0x0100
/// @param [in] hwnd    窗口句柄，NULL为删除主通道
/// @param [in] rows    划分行数目，1~16
/// @param [in] cols    划分列数目，1~16
/// @retval 1 成功
/// @retval 0 失败
XPR_API int XPR_MCVR_BindWindow(int port, void* hwnd, int rows, int cols);

/// @brief 向指定通道输入图像数据
/// @param [in] port       通道号（包含主通道号及子通道号）
/// @param [in] buf_data   图像数据指针
/// @param [in] buf_size   图像数据大小
/// @param [in] frame_info 图像帧信息
/// @retval 1 成功
/// @retval 0 失败
XPR_API int XPR_MCVR_InputData(int port, XPR_AVFrame* av_frame);

/// @brief 将源通道号图像放大到目标通道号
/// @param [in] src_port  源通道号
/// @param [in] dest_port 目标通道号
/// @retval 1 成功
/// @retval 0 失败
/// @par 目标通道号定义
///   Value  | Description
///   -------|-------------
///   -1     | 取消源通道的放大
///   0x0000 | 源通道所在主通道全区域放大
///   0xFFFF | 源通道所在主通道中心区域放大（暂时无效）
////  1~256  | 其他主通道全区域放大
XPR_API int XPR_MCVR_ZoomIn(int src_port, int dest_port);

/// @brief 获取指定子通道号的放大通道号
/// @param [in] port 源通道号
/// @return 放大通道号
/// @par 放大通道号定义
/// Value  | Description
/// -------|------------
/// -1     | 无放大
/// 0      | 所在主通道全区域放大
/// 0xFFFF | 所在主通道中心区域放大
/// 1~256  | 其他主通道全区域放大
XPR_API int XPR_MCVR_GetZoomPort(int port);

/// @brief 重置指定通道
/// @param [in] port 通道号
/// @par 主通道号为0，重置所有主通道下的所有子通道
///      主通道为有效通道号，子通道为0，重置指定主通道的所有子通道
///      主通道子通道都有效，重置指定子通道
/// @retval 1 成功
/// @retval 0 失败
XPR_API int XPR_MCVR_ResetPort(int port);

/// @brief 截取当前画面
/// @param [in] port 通道号
/// @param [in] path 截取文件路径
/// @retval 1 成功
/// @retval 0 失败
XPR_API int XPR_MCVR_Snapshot(int port, const char* path);

/// @brief 获取视频尺寸
/// @param [in] port 通道号
/// @param [in,out] width   视频宽度保存地址
/// @param [in,out] height  视频高度保存地址
/// @retval 1 成功
/// @retval 0 失败
XPR_API int XPR_MCVR_GetVideoSize(int port, int* width, int* height);

/// @brief 设置视频宽高比
/// @param [in] ratio 视频宽高比
/// @par 宽高比定义
/// Value                                | Description
/// -------------------------------------|------------------
/// XPR_MCVR_ASPECT_RATIO_ORIGINAL(0.0)  | 保持视频原有宽高比
/// 0.001 ~ 10.000                       | 指定宽高比
/// XPR_MCVR_ASPECT_RATIO_TILED(100.0)   | 铺满区域
/// @retval 1 成功
/// @retval 0 失败
XPR_API int XPR_MCVR_SetAspectRatio(float ratio);

/// @brief 获取视频宽高比
/// @return 当前视频宽高比
XPR_API float XPR_MCVR_GetAspectRatio(void);

/// @brief 设置视频显示比例
/// @param [in] port   通道号
/// @param [in] factor 显示放大比例，有效值: 1.000 ~ 16.000
/// @retval 1 成功
/// @retval 0 失败
XPR_API int XPR_MCVR_SetScale(int port, float factor);

/// @brief 获取视频显示放大比例
/// @param [in] port 通道号
/// @return 当前显示比例
XPR_API float XPR_MCVR_GetScale(int port);

/// @brief 设置渲染速率
/// @param [in] port 通道号
/// @param [in] rate 渲染速率
/// @retval 1 成功
/// @retval 0 失败
XPR_API int XPR_MCVR_SetRate(int port, float rate);

/// @brief 设置渲染级别
/// @param [in] level 渲染级别，参见 #XPR_MCVR_RenderLevel
/// @retval 1 成功
/// @retval 0 失败
XPR_API int XPR_MCVR_SetRenderLevel(XPR_MCVR_RenderLevel level);

/// @brief 设置渲染级别
/// @return 当前渲染级别
XPR_API XPR_MCVR_RenderLevel XPR_MCVR_GetRenderLevel(void);

/// @brief 重置单项式字符串表内容
/// @param [in] port    通道号
/// @param [in] type    字符串类型，参见 #XPR_MCVR_StringType
/// @param [in] strings 源字符串
/// @retval 1 成功
/// @retval 0 失败
/// @note 仅对XPR_MCVR_STRING_TYPE_TITLE类型字符串有效
XPR_API int XPR_MCVR_SetString(int port, XPR_MCVR_StringType type,
                               char* strings);

/// @brief 重置多项式字符串表内容
/// @param [in] port    通道号
/// @param [in] type    字符串类型，参见 #XPR_MCVR_StringType
/// @param [in] strings 源字符串
/// @param [in] count   字符串项目数
/// @retval 1 成功
/// @retval 0 失败
/// @note 仅对XPR_MCVR_STRING_TYPE_HINT类型字符串有效，且通道号被忽略
XPR_API int XPR_MCVR_SetStrings(int port, XPR_MCVR_StringType type,
                                char** strings, int count);

/// @brief 设置字符串显示状态
/// @param [in] port  通道号
/// @param [in] type  字符串类型，参见 #XPR_MCVR_StringType
/// @param [in] state 字符串显示状态，参见 #XPR_MCVR_StringState
/// @retval 1 成功
/// @retval 0 失败
/// @par 字符串类型及状态有效组合定义
/// State                        | XPR_MCVR_STRING_TYPE_TITLE | XPR_MCVR_STRING_TYPE_HINT 
/// -----------------------------|----------------------------|---------------------------
/// XPR_MCVR_STRING_STATE_HIDE   | Valid (Port is invalid)    | Valid (Port is invalid)
/// XPR_MCVR_STRING_STATE_SHOW   | Valid (Port is invalid)    | Valid (Port is invalid)
/// XPR_MCVR_STRING_STATE_SHOW_1 | Invalid                    | Valid (Port is Valid)
/// XPR_MCVR_STRING_STATE_SHOW_2 | Invalid                    | Valid (Port is Valid)
/// XPR_MCVR_STRING_STATE_SHOW_3 | Invalid                    | Valid (Port is Valid)
/// XPR_MCVR_STRING_STATE_SHOW_4 | Invalid                    | Valid (Port is Valid)
/// @par 多项式字符串状态对应 #XPR_MCVR_STRING_TYPE_HINT类型的定义
/// String State                 | Camera State
/// -----------------------------|---------------------
/// XPR_MCVR_STRING_STATE_SHOW_1 | No Device
/// XPR_MCVR_STRING_STATE_SHOW_2 | Buffering
/// XPR_MCVR_STRING_STATE_SHOW_3 | Disconnected
/// XPR_MCVR_STRING_STATE_SHOW_4 | Stopped
XPR_API int XPR_MCVR_SetStringState(int port, XPR_MCVR_StringType type,
                                    XPR_MCVR_StringState state);

/// @brief 获得字符串显示状态
/// @param [in] port 通道号
/// @param [in] type 字符串类型，参加 #XPR_MCVR_StringType
/// @return 当前字符串显示状态
XPR_API XPR_MCVR_StringState XPR_MCVR_GetStringState(int port,
                                                     XPR_MCVR_StringType type);

/// @brief 获得当前选中通道号
/// @return 当前通道号
XPR_API int XPR_MCVR_GetCurrentPort(void);

/// @brief 设置通道指定效果类型的值
/// @param [in] port   通道号
/// @param [in] effect 效果类型
/// @param [in] value  效果值
/// @retval 1 成功
/// @retval 0 失败
XPR_API int XPR_MCVR_SetEffect(int port, XPR_MCVR_EffectType effect, int value);

/// @brief 设置通道指定效果类型的值（浮点型）
/// @param [in] port
/// @param [in] effect
/// @param [in] value
/// @retval 1 成功
/// @retval 0 失败
XPR_API int XPR_MCVR_SetEffectF(int port, XPR_MCVR_EffectType effect,
                                float value);

/// @brief 获得通道指定效果类型的值
/// @param [in] port   通道号
/// @param [in] effect 效果类型
/// @return 效果值
XPR_API int XPR_MCVR_GetEffect(int port, XPR_MCVR_EffectType effect);

/// @brief 获得通道指定效果类型的值（浮点型）
/// @param [in] port
/// @param [in] effect
/// @return 效果值
XPR_API float XPR_MCVR_GetEffectF(int port, XPR_MCVR_EffectType effect);

/// @brief 事件回调函数定义
/// @param [in] ev   事件类型
/// @param [in] user 用户关联数据
/// @param [in] port 通道号
/// @param [in] data 事件附带数据
/// @retval 1 成功
/// @retval 0 失败
/// @par 附带数据定义
///   Event Type            | Data type | Range                 | Description
///   ----------------------|-----------|-----------------------|-----------
///   XPR_MCVR_LEFT_CLICK   | int*      | Depends               | 点击坐标，低字为X坐标，高字为Y坐标
///   XPR_MCVR_RIGHT_CLICK  | int*      | Depends               | 点击坐标，低字为X坐标，高字为Y坐标
///   XPR_MCVR_LEFT_DBCLICK | int*      | Depends               | 点击坐标，低字为X坐标，高字为Y坐标
typedef int (*XPR_MCVR_EventCallback)(XPR_MCVR_EventType ev, void* user,
                                      int port, const void* data);

/// @brief 注册事件回调函数
/// @param [in] callback 事件回调函数
/// @param [in] user     用户关联数据
/// @retval 1 成功
/// @retval 0 失败
/// @sa XPR_MCVR_AttachEvent(), XPR_MCVR_DetachEvent()
XPR_API int XPR_MCVR_AddEventCallback(XPR_MCVR_EventCallback callback,
                                      void* user);

/// @brief 注销事件回调函数
/// @param [in] callback 事件回调函数
/// @param [in] user     用户关联数据
/// @retval 1 成功
/// @retval 0 失败
/// @sa XPR_MCVR_AttachEvent(), XPR_MCVR_DetachEvent()
XPR_API int XPR_MCVR_DelEventCallback(XPR_MCVR_EventCallback callback,
                                      void* user);

/// @brief 添加需关注的事件
/// @param [in] ev 事件类型，参加 #XPR_MCVR_EventType
/// @retval 1 成功
/// @retval 0 失败
/// @sa XPR_MCVR_AddEventCallback(), XPR_MCVR_DelEventCallback()
XPR_API int XPR_MCVR_AttachEvent(XPR_MCVR_EventType ev);

/// @brief 关注所有事件
/// @param [in] port 通道号，预留参数，仅对将来可能添加的事件有效
/// @retval 1 成功
/// @retval 0 失败
/// @sa XPR_MCVR_DetachAllEvents()
XPR_API int XPR_MCVR_AttachAllEvents(void);

/// @brief 撤销已关注的事件
/// @param [in] port  通道号，预留参数，仅对将来可能添加的事件有效
/// @param [in] ev 事件类型，参加 #XPR_MCVR_EventType
/// @retval 1 成功
/// @retval 0 失败
/// @sa XPR_MCVR_AddEventCallback(), XPR_MCVR_DelEventCallback()
XPR_API int XPR_MCVR_DetachEvent(XPR_MCVR_EventType ev);

/// @brief 撤销所有已关注事件
/// @param [in] port 通道号，预留参数，仅对将来可能添加的事件有效
/// @retval 1 成功
/// @retval 0 失败
/// @sa XPR_MCVR_AttachAllEvents()
XPR_API int XPR_MCVR_DetachAllEvents(void);

/// @brief  获得最新错误码
/// @return 当前错误码，参加 #_XPR_MCVR_ErrorType
XPR_API int XPR_MCVR_GetLastError(void); 

/// @brief  获得最新错误字符串描述(多字节)
/// @return 当前错误字符串(调用者需要释放该字符串资源)(XPR_Free或XPR_Feeep)
XPR_API char *XPR_MCVR_GetErrorString(void);

/// @brief  获得最新错误字符串描述(宽字符)
/// @return 当前错误字符串(调用者需要释放该字符串资源)(XPR_Free或XPR_Feeep)
XPR_API wchar_t *XPR_MCVR_GetErrorStringW(void);

#ifdef __cplusplus
}
#endif

#endif // XPR_MCVR
