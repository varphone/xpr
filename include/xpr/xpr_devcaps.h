/*
 * File: xpr_devcaps.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 设备能力操作接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2014-11-21 12:50:43 Friday, 21 November
 * Last Modified : 2019-07-03 05:24:14 Wednesday, 3 July
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
#ifndef XPR_DEVCAPS_H
#define XPR_DEVCAPS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

///
/// 所有设备
///
#define XPR_DEVCAPS_DEVID_ALL   0xffff

///
/// 任一设备
///
#define XPR_DEVCAPS_DEVID_ANY   0xfffe

///
/// 默认设备
///
#define XPR_DEVCAPS_DEVID_DEF   0x0000

#define XPR_DEVCAPS_VERSION(major,minor,rev)        ((((uint32_t)major) << 16) | (((uint32_t)minor) << 8) | (rev))
#define XPR_DEVCAPS_VERSION_MAJOR(v)                (((v) >> 16) & 0xffff)
#define XPR_DEVCAPS_VERSION_MINOR(v)                (((v) >> 8) & 0xff)
#define XPR_DEVCAPS_VERSION_REV(v)                  ((v) & 0xff)

#ifndef XPR_DEVCAPSID_TYPE_DEFINED
#define XPR_DEVCAPSID_TYPE_DEFINED
///
/// 设备能力编号定义
///
typedef enum XPR_DevCapsId {
    XPR_DEVCAPS_ID_UNKNOWN,         ///< 未知
    XPR_DEVCAPS_ID_VERSION,         ///< 设备能力数据所属版本, (uint32_t){major[31..16],minor[15..8],rev:[7..0]}
    XPR_DEVCAPS_ID_DATETIME,        ///< 设备能力数据生成日期, (int64_t)
    XPR_DEVCAPS_ID_INTERNAL_MODEL,  ///< 内部型号, (const char*){64 bytes}
    XPR_DEVCAPS_ID_MODEL,           ///< 出厂型号, (const char*){64 bytes}
    XPR_DEVCAPS_ID_HW_VER,          ///< 硬件版本, (uint32_t){major[31..16],minor[15..8],rev:[7..0]}
    XPR_DEVCAPS_ID_SW_VER,          ///< 软件版本, (uint32_t){major[31..16],minor[15..8],rev:[7..0]}
    XPR_DEVCAPS_ID_LIGHT_SENSOR,    ///< 光照传感器, (uint8_t){0=无,1=普通光敏电阻}
    XPR_DEVCAPS_ID_THERM_SENSOR,    ///< 热能传感器, (uint8_t){0=无,1=普通温度传感器}
    XPR_DEVCAPS_ID_VIDEO_SENSOR,    ///< 图像传感器名称, (const char*)｛16 bytes}
    XPR_DEVCAPS_ID_IRCUT,           ///< 红外滤光片, (uint8_t){0=无,1=固定,2=可控}
    XPR_DEVCAPS_ID_AUX_LIGHT,       ///< 辅助光源, (uint8_t){0=无,1=白光灯,2=红外灯}
    XPR_DEVCAPS_ID_AF,              ///< 自动聚集, (uint8_t){0=无,1=普通自动聚集}
    XPR_DEVCAPS_ID_LENS,            ///< 镜头, (uint8_t){0=无,1=全手动,2=手动聚焦+电动光圈,
                                    //         3=电动聚焦+手动光圈,4=电动聚焦+电动光圈,5=全自动聚焦+电动光圈}
    XPR_DEVCAPS_ID_VENC_CODECS,     ///< 视频编码器编码格式, (uint32_t){h264[0],h265[1],mjpeg[2],mpeg4[3]}
    XPR_DEVCAPS_ID_VENC_MAX_RES,    ///< 视频编码器最大分辨率, (uint32_t){width[31..16],height[15..0]}
    XPR_DEVCAPS_ID_VENC_MIN_RES,    ///< 视频编码器最小分辨率, (uint32_t){width[31..16],height[15..0]}
    XPR_DEVCAPS_ID_MAX,
} XPR_DevCapsId;
#endif // XPR_DEVCAPSID_TYPE_DEFINED

///  
/// 初始化 XPR_DEVCAPS 模块
///
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_DevCapsInit(void);

///
/// 释放 XPR_DEVCAPS 模块已分配的资源
///
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_DevCapsFini(void);

///
/// 获取整型设备能力
///
/// @param [in] capId       设备能力编号, 见 [#XPR_DevCapsId]
/// @param [in] devId       设备编号, 见 [#XPR_DEVCAPS_DEVID_ALL]
/// @return 返回整数值
int XPR_DevCapsGetInteger(int capId, int devId);

///
/// 获取 64 位整型设备能力
///
/// @param [in] capId       设备能力编号, 见 [#XPR_DevCapsId]
/// @param [in] devId       设备编号, 见 [#XPR_DEVCAPS_DEVID_ALL]
/// @return 返回 64 位整数值
int64_t XPR_DevCapsGetInt64(int capId, int devId);

///
/// 获取实数型设备能力
///
/// @param [in] capId       设备能力编号, 见 [#XPR_DevCapsId]
/// @param [in] devId       设备编号, 见 [#XPR_DEVCAPS_DEVID_ALL]
/// @return 返回实数值
double XPR_DevCapsGetReal(int capId, int devId);

///
/// 获取字串形设备能力
///
/// @param [in] capId       设备能力编号, 见 [#XPR_DevCapsId]
/// @param [in] devId       设备编号, 见 [#XPR_DEVCAPS_DEVID_ALL]
/// @return 返回字串指针
const char* XPR_DevCapsGetString(int capId, int devId);

///
/// 获取字串表形设备能力
///
/// @param [in] capId       设备能力编号, 见 [#XPR_DevCapsId]
/// @param [in] devId       设备编号, 见 [#XPR_DEVCAPS_DEVID_ALL]
/// @return 返回字串表指针
/// @note 字串表格式 ["str1", "str2", "str3", "strN", NULL]
const char** XPR_DevCapsGetStrings(int capId, int devId);

///
/// 更新设备能力数据
///
/// @param [in] data        设备能力数据
/// @param [in] size        设备能力数据字节数
/// @retval XPR_ERR_OK      成功
/// @retval XPR_ERR_ERROR   失败
int XPR_DevCapsUpdate(const void* data, int size);

#define XPR_DevCapsGetVersion()                                                \
    XPR_DevCapsGetInteger(XPR_DEVCAPS_ID_VERSION, XPR_DEVCAPS_DEVID_DEF)
#define XPR_DevCapsGetDateTime()                                               \
    XPR_DevCapsGetInt64(XPR_DEVCAPS_ID_DATETIME, XPR_DEVCAPS_DEVID_DEF)

#ifdef __cplusplus
}
#endif

#endif // XPR_DEVCAPS_H
