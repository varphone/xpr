/*
 * File: xpr_osd.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 通用 OSD 绘制接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2014-12-29 04:03:10 Monday, 29 December
 * Last Modified : 2019-07-03 04:52:23 Wednesday, 3 July
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
#ifndef XPR_OSD_H
#define XPR_OSD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 前置声明
typedef struct XPR_Image XPR_Image;
typedef struct XPR_Rect XPR_Rect;

//==============================================================================
// PORT 的解析
//
// 本库的所有操作都是基于 PORT 来实现, PORT 的意思是端口, 在本库中代表着目标、通道的含义
// PORT 有 2 部分构成：
//     major   主目标, 用来区分实例化目标类型, 1 = 视频编码 OSD 目标
//     minor   次目标, 用来指定实例化目标
//
// 例如：
//     PORT = 0x00010000 = 视频编码 OSD 对象
//     PORT = 0x00010001 = 视频编码 OSD 对象的第一个区域
//
// 每部分都有 3 个值是有特殊含义的:
//     0    表示此部分被忽略
//     M    表示所有目标
//     M-1  表示可以目标中的任一
// * (M) 是每部分的最大值, 例如 major 是 4 个位, 其最大值是 0xF
//
//==============================================================================

///
/// 合成 OSD 对象句柄
///
/// @param [in] major       主目标编号
/// @param [in] minor       次目标编号
/// @return OSD 对象句柄
#define XPR_OSD_PORT(major, minor)  (((major) << 16) | (minor))

#define XPR_OSD_PORT_MAJOR(a)       ((a)>>8 & 0xffff)
#define XPR_OSD_PORT_MINOR(a)       ((a) & 0xffff)
#define XPR_OSD_PORT_MAJOR_MASK     0xffff0000
#define XPR_OSD_PORT_MINOR_MASK     0x0000ffff

#define XPR_OSD_PORT_MAJOR_ALL      0xffff
#define XPR_OSD_PORT_MAJOR_ANY      0xfffe
#define XPR_OSD_PORT_MAJOR_DEF      0x0001
#define XPR_OSD_PORT_MAJOR_MIN      0x0001
#define XPR_OSD_PORT_MAJOR_MAX      0x000f
#define XPR_OSD_PORT_MAJOR_NUL      0x0000

#define XPR_OSD_PORT_MINOR_ALL      0xffff
#define XPR_OSD_PORT_MINOR_ANY      0xfffe
#define XPR_OSD_PORT_MINOR_DEF      0x0001
#define XPR_OSD_PORT_MINOR_MIN      0x0001
#define XPR_OSD_PORT_MINOR_MAX      0x000f
#define XPR_OSD_PORT_MINOR_NUL      0x0000

#define XPR_OSD_AREA1               XPR_OSD_PORT(XPR_OSD_PORT_MAJOR_DEF, XPR_OSD_PORT_MINOR_MIN)
#define XPR_OSD_AREA2               XPR_OSD_PORT(XPR_OSD_PORT_MAJOR_DEF, XPR_OSD_PORT_MINOR_MIN+1)
#define XPR_OSD_AREA3               XPR_OSD_PORT(XPR_OSD_PORT_MAJOR_DEF, XPR_OSD_PORT_MINOR_MIN+2)
#define XPR_OSD_AREA4               XPR_OSD_PORT(XPR_OSD_PORT_MAJOR_DEF, XPR_OSD_PORT_MINOR_MIN+3)

#ifndef XPR_OSD_TEXTALIGNMENT_TYPE_DEFINED
#define XPR_OSD_TEXTALIGNMENT_TYPE_DEFINED
///
/// OSD 文本对齐类型定义
///
typedef enum XPR_OSD_TextAlignment {
    XPR_OSD_TEXT_ALIGN_LEFT     = 0x01,     ///< 左对齐
    XPR_OSD_TEXT_ALIGN_TOP      = 0x02,     ///< 顶对齐
    XPR_OSD_TEXT_ALIGN_RIGHT    = 0x04,     ///< 右对齐
    XPR_OSD_TEXT_ALIGN_BOTTOM   = 0x08,     ///< 底对齐
    XPR_OSD_TEXT_ALIGN_CENTER   = 0x10,     ///< 水平居中
    XPR_OSD_TEXT_ALIGN_VCENTER  = 0x20,     ///< 垂直居中
} XPR_OSD_TextAlignment;
#endif // XPR_OSD_TEXTALIGNMENT_TYPE_DEFINED

///
/// 初始化 XPR_OSD 模块
///
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_Init(void);

/// 释放 XPR_OSD 模块已分配的资源
///
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_Fini(void);

///
/// 打开指定目标
///
/// @param [in] port        OSD 目标端口
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_Open(int port);

///
/// 关闭指定目标
///
/// @param [in] port        OSD 目标端口
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_Close(int port);

///
/// 检测端口是否有效
///
/// @param [in] port        OSD 目标端口
/// @retval XPR_TRUE 有效
/// @retval XPR_FALSE 无效
int XPR_OSD_IsPortValid(int port);

///
/// 获取目标背景色(ARGB)
///
/// @param [in] port        OSD 目标端口
/// @param [in] color       保存颜色值的地址
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_GetBGColor(int port, unsigned int* color);

///
/// 设置目标背景色(ARGB)
///
/// @param [in] port        OSD 目标端口
/// @param [in] color       颜色值
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_SetBGColor(int port, unsigned int color);

///
/// 获取目标前景色(ARGB)
///
/// @param [in] port        OSD 目标端口
/// @param [in] color       保存颜色值的地址
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_GetFGColor(int port, unsigned int* color);

///
/// 设置目标前景色(ARGB)
///
/// @param [in] port        OSD 目标端口
/// @param [in] color       颜色值
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_SetFGColor(int port, unsigned int color);

///
/// 获取目标边框色(ARGB)
///
/// @param [in] port        OSD 目标端口
/// @param [in] color       保存颜色值的地址
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_GetBorderColor(int port, unsigned int* color);

///
/// 设置目标边框色(ARGB)
///
/// @param [in] port        OSD 目标端口
/// @param [in] color       颜色值
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_SetBorderColor(int port, unsigned int color);

///
/// 获取目标偏移
///
/// @param [in] port        OSD 目标端口
/// @param [in] left        保存左偏移值的地址
/// @param [in] top         保存顶偏移值的地址
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_GetOffset(int port, int* left, int* top);

///
/// 设置目标偏移
///
/// @param [in] port        OSD 目标端口
/// @param [in] left        左偏移值
/// @param [in] top         顶偏移值
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_SetOffset(int port, int left, int top);

///
/// 获取目标区域属性
///
/// @param [in] port        OSD 目标端口
/// @param [in,out] rect    保存区域属性的地址
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_GetRect(int port, XPR_Rect* rect);

///
/// 获取目标区域属性
///
/// @param [in] port        OSD 目标端口
/// @param [in] rect        区域属性
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_SetRect(int port, const XPR_Rect* rect);

///
/// 获取目标尺寸
///
/// @param [in] port        OSD 目标端口
/// @param [in,out] width   保存宽度值的地址
/// @param [in,out] height  保存高度值的地址
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_GetSize(int port, int* width, int* height);

///
/// 设置目标尺寸
///
/// @param [in] port        OSD 目标端口
/// @param [in] width       保存宽度值的地址
/// @param [in] height      保存高度值的地址
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_SetSize(int port, int width, int height);

///
/// 清除目标内容并填充为指定颜色
///
/// @param [in] port        OSD 目标端口
/// @param [in] rect        目标区域, 为 NULL 时表示整个目标
/// @param [in] color       填充颜色值
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_Clear(int port, const XPR_Rect* rect, unsigned int color);

///
/// 绘制图像
///
/// @param [in] port        OSD 目标端口
/// @param [in] rect        目标区域, 为 NULL 时表示整个目标
/// @param [in] image       图像数据
/// @param [in] flags       绘制标志
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_DrawImage(int port, const XPR_Rect* rect, const XPR_Image* image,
                      int flags);

///
/// 绘制矩形
///
/// @param [in] port        OSD 目标端口
/// @param [in] rect        目标区域, 为 NULL 时表示整个目标
/// @param [in] image       图像数据
/// @param [in] color       矩形颜色
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_DrawRect(int port, const XPR_Rect* rect, unsigned int color);

///
/// 绘制文本
///
/// @param [in] port        OSD 目标端口
/// @param [in] rect        目标区域, 为 NULL 时表示整个目标
/// @param [in] text        文本数据
/// @param [in] flags       绘制标志
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_DrawText(int port, const XPR_Rect* rect, const char* text,
                     unsigned int flags);

///
/// 填充矩形
///
/// @param [in] port        OSD 目标端口
/// @param [in] rect        目标区域, 为 NULL 时表示整个目标
/// @param [in] color       填充颜色
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_FillRect(int port, const XPR_Rect* rect, unsigned int color);

///
/// 更新 OSD 数据到最终目标
///
/// @param [in] port        OSD 目标端口
/// @param [in] rect        目标区域, 为 NULL 时表示整个目标
/// @retval XPR_ERR_OK 成功
/// @retval XPR_ERR_ERROR 失败
int XPR_OSD_Update(int port, const XPR_Rect* rect);

#ifdef __cplusplus
}
#endif

#endif // XPR_OSD_H
