#ifndef XPR_MCDEC_H
#define XPR_MCDEC_H

#include <stdint.h>
#include "xpr_avframe.h"
#include "xpr_streamblock.h"

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// PORT 的解析
//
// 本库的所有操作都是基于 PORT 来实现, PORT 的意思是端口, 在本库中代表着目标、通道的含义
// PORT 有 2 部分构成：
//     major   主目标, 用来区分 解码器 组
//     minor   次目标, 用来指定 解码器 组中特定目标
//
// 例如：
//     PORT = 0x00010000 = 第一个 解码器 组
//     PORT = 0x00010001 = 第一个 解码器 组中第一个端口
//
// 每部分都有 3 个值是有特殊含义的:
//     0    表示此部分被忽略
//     M    表示所有目标
//     M-1  表示可以目标中的任一
// * (M) 是每部分的最大值, 例如 major 是 16 个位, 其最大值是 0xFFFF
//
//==============================================================================

///
/// 合成 解码器 端口号
///
#define XPR_MCDEC_PORT(major, minor)     (((major)<<16)|(minor))

///
/// 提取 解码器 端口号中主目标
///
#define XPR_MCDEC_PORT_MAJOR(port)       (((port)>>16) & 0xffff)

///
/// 提取 解码器 端口号中次目标
///
#define XPR_MCDEC_PORT_MINOR(port)       ((port) & 0xffff)

#define XPR_MCDEC_PORT_MAJOR_ALL         0xffff
#define XPR_MCDEC_PORT_MAJOR_ANY         0xfffe
#define XPR_MCDEC_PORT_MAJOR_DEF         0x0001
#define XPR_MCDEC_PORT_MAJOR_NUL         0x0000
#define XPR_MCDEC_PORT_MAJOR_MIN         0x0001
#define XPR_MCDEC_PORT_MAJOR_MAX         0xfffd

///
/// 通用数据解码器组
///
#define XPR_MCDEC_PORT_MAJOR_GEN         0x0001

#define XPR_MCDEC_PORT_MINOR_ALL         0xffff
#define XPR_MCDEC_PORT_MINOR_ANY         0xfffe
#define XPR_MCDEC_PORT_MINOR_DEF         0x0001
#define XPR_MCDEC_PORT_MINOR_NUL         0x0000
#define XPR_MCDEC_PORT_MINOR_MIN         0x0001
#define XPR_MCDEC_PORT_MINOR_MAX         0xfffd

///
/// 特殊端口：BMP 格式编码器
///
#define XPR_MCDEC_SPEC_BMPENC_PORT      (0x00000001)

///
/// 特殊端口：JPG 格式编码器
///
#define XPR_MCDEC_SPEC_JPGENC_PORT      (0x00000002)

///
/// 特殊端口：PNG 格式编码器
///
#define XPR_MCDEC_SPEC_PNGENC_PORT      (0x00000003)

///
/// 单个端口可容纳的最大回调函数总量
///
#define XPR_MCDEC_MAX_HANDLERS          8

///
/// 音/视频原始帧数据回调函数
///
/// @param [in] opaque		用户关联数据
/// @param [in] port		产生回调的端口
/// @param [in] avf			音/视频原始帧数据
/// return 目前忽略返回值
typedef int (*XPR_MCDEC_AVF_FXN)(void* opaque, int port, const XPR_AVFrame* avf);

///
/// 音/视频码流帧数据回调函数
///
/// @param [in] opaque		用户关联数据
/// @param [in] port		产生回调的端口
/// @param [in] stb			音/视频码流帧数据
/// return 目前忽略返回值
typedef int (*XPR_MCDEC_STB_FXN)(void* opaque, int port, const XPR_StreamBlock* stb);

///
/// Print no output.
///
#define XPR_MCDEC_LOG_LEVEL_QUIET    -8

///
/// Something went really wrong and we will crash now.
///
#define XPR_MCDEC_LOG_LEVEL_PANIC     0

///
/// Something went wrong and recovery is not possible.
/// For example, no header was found for a format which depends
/// on headers or an illegal combination of parameters is used.
///
#define XPR_MCDEC_LOG_LEVEL_FATAL     8

///
/// Something went wrong and cannot losslessly be recovered.
/// However, not all future data is affected.
///
#define XPR_MCDEC_LOG_LEVEL_ERROR    16

///
/// Something somehow does not look correct. This may or may not
/// lead to problems. An example would be the use of '-vstrict -2'.
///
#define XPR_MCDEC_LOG_LEVEL_WARNING  24

///
/// Standard information.
///
#define XPR_MCDEC_LOG_LEVEL_INFO     32

///
/// Detailed information.
///
#define XPR_MCDEC_LOG_LEVEL_VERBOSE  40

///
/// Stuff which is only useful for libav* developers.
///
#define XPR_MCDEC_LOG_LEVEL_DEBUG    48

typedef enum XPR_MCDEC_CFG {
	XPR_MCDEC_CFG_C4WGA,
	XPR_MCDEC_CFG_LOG_LEVEL,		///< 日志打印等级, 数据类型: int, 值: 见 XPR_MCDEC_LOG_LEVEL_xxx 
    XPR_MCDEC_CFG_MAX,
} XPR_MCDEC_CFG;

typedef enum XPR_MCDEC_FLAG {
	XPR_MCDEC_FLAG_BLKED = 0x00000001,
	XPR_MCDEC_FLAG_CLOSE = 0x00000002,
	XPR_MCDEC_FLAG_PNDNG = 0x00000004,
	XPR_MCDEC_FLAG_SYNRQ = 0x00000008,
} XPR_MCDEC_FLAG;

typedef enum XPR_MCDEC_PARAM {
	XPR_MCDEC_PARAM_NULL,
	XPR_MCDEC_PARAM_BLOCK,			///< data: int, value: 0 = unblock, 1 = block
	XPR_MCDEC_PARAM_DEINTERLANCE,	///< data: int, value: 0 = off, 1 = odd, 2 = even
	XPR_MCDEC_PARAM_SKIP_FRAME,		///< data: int, value: 0 = none, 1 = B, 2 = P, 3 = BP, 4 = I, 5 = BI, 6 = PI, 7 = BPI
	XPR_MCDEC_PARAM_FORCE_FOURCC,	///< data: uint32_t, value: FourCC
    XPR_MCDEC_PARAM_IMAGE_WIDTH,
    XPR_MCDEC_PARAM_IMAGE_HEIGHT,
    XPR_MCDEC_PARAM_IMAGE_QUALITY,
	XPR_MCDEC_PARAM_MAX,
} XPR_MCDEC_PARAM;

///
/// 配置整个多通道解码器
///
/// @param [in] cfg         配置选项编码, 参见 [#XPR_MCDEC_CFG]
/// @param [in] data        配置数据
/// @param [in] size        配置数据长度, NULL 结尾的字串型数据可以为 -1
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_MCDEC_Config(XPR_MCDEC_CFG cfg, const void* data, int size);

///
/// 初始化整个多通道解码器
///
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_MCDEC_Init(void);

///
/// 释放整个多通道解码器
///
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_MCDEC_Fini(void);

///
/// 立即清洗解码缓冲区
///
/// @param [in] port        要操作的端口
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_MCDEC_Flush(int port);

///
/// 重置指定端口并释放其所分配的一切资源
///
/// @param [in] port        要操作的端口
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_MCDEC_Reset(int port);

/// 向指定端口推入一帧原始音/视频帧数据
///
/// @param [in] port        要操作的端口
/// @param [in] avf         音/视频原始帧数据
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_MCDEC_PushAVFrame(int port, const XPR_AVFrame* avf);

///
/// 向指定端口推入一帧原始音/视频码流数据
///
/// @param [in] port        要操作的端口
/// @param [in] stb         音/视频码流数据
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_MCDEC_PushStreamBlock(int port, const XPR_StreamBlock* stb);

///
/// 向指定端口推入没有容器包装的数据
///
/// @param [in] port        要操作的端口
/// @param [in] data        要处理的数据, 其格式需要使用 XPR_MCDEC_SetParam() 进行协商
/// @param [in] length      要处理的数据的字节数
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_MCDEC_PushData(int port, const uint8_t* data, int length);

///
/// 分发解码后的数据
/// @param [in] port        要操作的端口
/// @param [in] avf         音/视频原始帧数据
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
void XPR_MCDEC_DeliverAVFrame(int port, const XPR_AVFrame* avf);

///
/// 分发编码/转码后的数据
/// @param [in] port        要操作的端口
/// @param [in] stb         音/视频码流数据
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
void XPR_MCDEC_DeliverStreamBlock(int port, const XPR_StreamBlock* stb);

///
/// 向指定端口添加处理音/视频原始帧数据的回调函数
///
/// @param [in] port        要操作的端口
/// @param [in] handler     回调函数
/// @param [in] opaque      用关联数据
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
/// @note 此接口端口可以为：编码器组或编码器组中的特定目标
///       当端口为 编码器组 时, 编码器组中的所有目标都会调用此回调函数
int XPR_MCDEC_AddAVFrameHandler(int port, XPR_MCDEC_AVF_FXN handler, void* opaque);

int XPR_MCDEC_DelAVFrameHandler(int port, XPR_MCDEC_AVF_FXN handler, void* opaque);

///
/// 向指定端口添加处理音/视频码流数据的回调函数
///
/// @param [in] port        要操作的端口
/// @param [in] handler     回调函数
/// @param [in] opaque      用关联数据
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
/// @note 此接口端口可以为：编码器组或编码器组中的特定目标
///       当端口为 编码器组 时, 编码器组中的所有目标都会调用此回调函数
int XPR_MCDEC_AddStreamBlockHandler(int port, XPR_MCDEC_STB_FXN handler, void* opaque);

int XPR_MCDEC_DelStreamBlockHandler(int port, XPR_MCDEC_STB_FXN handler, void* opaque);

///
/// 设置指定端口的参数
/// @param [in] port        要操作的端口
/// @param [in] param       要设置的参数编号, 参见 [#XPR_MCDEC_Param]
/// @param [in] data        要设置的参数值/数据
/// @param [in] size        要设置的参数值/数据字节数
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_MCDEC_SetParam(int port, XPR_MCDEC_PARAM param, const void* data, int size);

///
/// 获取指定端口的参数
/// @param [in] port        要操作的端口
/// @param [in] param       要获取的参数编号, 参见 [#XPR_MCDEC_Param]
/// @param [in] data        要获取的参数值/数据接收缓冲区
/// @param [in,out] size    要获取的参数值/数据接收缓冲区字节数, 并返回实际数据长度
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_MCDEC_GetParam(int port, XPR_MCDEC_PARAM param, void* data, int* size);

#ifdef __cplusplus
}
#endif

#endif // XPR_MCDEC_H

