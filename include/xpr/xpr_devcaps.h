#ifndef XPR_DEVCAPS_H
#define XPR_DEVCAPS_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C"
{
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
#define XPR_DEVCAPS_DEVID_0   XPR_DEVCAPS_DEVID_DEF
#define XPR_DEVCAPS_DEVID_1   0x0001
#define XPR_DEVCAPS_DEVID_2   0x0002
#define XPR_DEVCAPS_DEVID_3   0x0003


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
    XPR_DEVCAPS_ID_UNKNOWN          = 0x0000,///< 未知
    XPR_DEVCAPS_ID_VERSION,                 //< 设备能力数据所属版本, (uint32_t){major[31..16],minor[15..8],rev:[7..0]}
    XPR_DEVCAPS_ID_DATETIME,                ///< 设备能力数据生成日期, (int64_t)
    XPR_DEVCAPS_ID_INTERNAL_MODEL,          ///< 内部型号, (const char*){64 bytes}
    XPR_DEVCAPS_ID_MODEL,                   ///< 出厂型号, (const char*){64 bytes}
    XPR_DEVCAPS_ID_SN,                      ///< 序列号, (const char*){32 bytes}
    XPR_DEVCAPS_ID_HW_VER,                  ///< 硬件版本, (uint32_t){major[31..16],minor[15..8],rev:[7..0]}
    XPR_DEVCAPS_ID_SW_VER,                  ///< 软件版本, (uint32_t){major[31..16],minor[15..8],rev:[7..0]}
    XPR_DEVCAPS_ID_LIGHT_SENSOR,            ///< 光照传感器, (uint8_t){0=无,1=普通光敏电阻}
    XPR_DEVCAPS_ID_THERM_SENSOR,            ///< 热能传感器, (uint8_t){0=无,1=普通温度传感器}
    XPR_DEVCAPS_ID_VIDEO_SENSOR,            ///< 图像传感器名称, (const char*)｛16 bytes}
    XPR_DEVCAPS_ID_IRCUT,                   ///< 红外滤光片, (uint8_t){0=无,1=固定,2=可控}
    XPR_DEVCAPS_ID_AUX_LIGHT,               ///< 辅助光源, (uint8_t){0=无,1=白光灯,2=红外灯}
    XPR_DEVCAPS_ID_AF,                      ///< 自动聚集, (uint8_t){0=无,1=普通自动聚集}
    XPR_DEVCAPS_ID_LENS,                    ///< 镜头, (uint8_t){0=无,1=全手动,2=手动聚焦+电动光圈,
                                            //         3=电动聚焦+手动光圈,4=电动聚焦+电动光圈,5=全自动聚焦+电动光圈}
    XPR_DEVCAPS_ID_VENC_MAX_RES,            ///< 最大分辨率 (uint32_t){width[31..16],height[15..0]}
    XPR_DEVCAPS_ID_VENC_MAX_FRAME,      ///< 最大帧率
    XPR_DEVCAPS_ID_VENC_MAX_BITRATES,       ///< 最大位率 (uint32_t){MAX[31..16],MIN[15..0]}
    XPR_DEVCAPS_ID_AUDIO_INPUT,             ///< 音频输入 0 不支持 1支持
    XPR_DEVCAPS_ID_AUDIO_OUTPUT,            ///< 音频输出 0 不支持 1 单通道 2 双通道
    XPR_DEVCAPS_ID_VENC_CODECS     = 0x4000,  ///< 视频编码格式 uint8_t （32bytes） "h264,h265,mpeg4"
    XPR_DEVCAPS_ID_VENC_RESOLUTION,           ///< 分辨率 uint8_t (96 bytes) "4K,5M,1920X1080,1280X720,...."
    XPR_DEVCAPS_ID_PTZ,                       ///< 云台 uint8_t "common"
    XPR_DEVCAPS_ID_GIS,                       ///< 地理信息系统 uint8_t （32bytes）“GPS,Compass,Gyro,Ldm"
    XPR_DEVCAPS_ID_EVENT,                     ///< 事件 uint8_t "motion,cover"
    XPR_DEVCAPS_ID_AUDIO_CODECS,              ///< 视频编码格式 uint8_t （32bytes） "G.711,G.726"
    XPR_DEVCAPS_ID_PORT_TYPE,                 ///< 白平衡模式 uint8_t （32bytes） "RS485/RS232"
    XPR_DEVCAPS_ID_VENC_STREAM_COUNTS,        ///< 支持几路视频流通道 uint8_t 1=1路，2=2路，...
    XPR_DEVCAPS_ID_VENC_QUALITY,              ///< 视频质量 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_VENC_FRAME,                ///< 图像帧率 uint32_t {max[31..16],min[15..0]}
    XPR_DEVCAPS_ID_IRIS,                      ///< 光圈模式 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_SHUTTER_TIME,              ///< 电子快门时间 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_EXPOSURE_LEVEL,            ///< 光圈模式 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_SHUTTER_MAX_TIME,          ///< 电子快门最大时间 uint8_t max[31..16],min[15..0]
    XPR_DEVCAPS_ID_SHUTTER_MIN_TIME,          ///< 电子快门最小时间 uint8_t max[31..16],min[15..0]
    XPR_DEVCAPS_ID_NOISE_2D,                  ///< 2D降噪 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_NOISE_3D,                  ///< 3D降噪 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_GRAY_SCALE,                ///< 灰度模式 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_WHITE_BALANCE,             ///< 白平衡 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_MIRROR,                    ///< 镜像模式 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_POWER_LINE,                ///< 视频制式 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_DAY_NIGHT,                 ///< 日夜模式 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_LIGHT,                     ///< 补光灯类型 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_LIGHT_MODE,                ///< 联动模式 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_WDR,                       ///< 宽动态 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_FLOG,                      ///< 透雾 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_METER,                     ///< 测光 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_STABILIZATION,             ///< 防抖 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_LENS_CORRECTION,           ///< 镜头校正 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_PORTS,                     ///< 端口数 uint8_t 1=1个,2=2个
    XPR_DEVCAPS_ID_BAUD_RATE,                 ///< 波特率 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_DATA_BITS,                 ///< 数据位 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_STOP_BITS,                 ///< 停止位 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_PARITY_CHECK,              ///< 校验 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_FLOW_CONTROL,              ///< 流控 uint8_t 0=无,1=支持
    XPR_DEVCAPS_ID_EVENT_MOTION_SENSITIVE,    ///< 移动侦测灵敏度 (uint32_t){max[31..16],min[15..0]}
    XPR_DEVCAPS_ID_EVENT_COVER_SENSITIVE      ///< 遮挡报警灵敏度 (uint32_t){max[31..16],min[15..0]}
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
int XPR_DevCapsGetInteger(int capId, int devId, uint32_t *value);

///
/// 获取 64 位整型设备能力
///
/// @param [in] capId       设备能力编号, 见 [#XPR_DevCapsId]
/// @param [in] devId       设备编号, 见 [#XPR_DEVCAPS_DEVID_ALL]
/// @return 返回 64 位整数值
int  XPR_DevCapsGetInt64(int capId, int devId, int64_t *value);

///
/// 获取实数型设备能力
///
/// @param [in] capId       设备能力编号, 见 [#XPR_DevCapsId]
/// @param [in] devId       设备编号, 见 [#XPR_DEVCAPS_DEVID_ALL]
/// @return 返回实数值
int  XPR_DevCapsGetReal(int capId, int devId, double *value);

///
/// 获取字串形设备能力
///
/// @param [in] capId       设备能力编号, 见 [#XPR_DevCapsId]
/// @param [in] devId       设备编号, 见 [#XPR_DEVCAPS_DEVID_ALL]
/// @return 返回字串指针
int  XPR_DevCapsGetString(int capId, int devId, char* buffer, int len);

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

#define XPR_DevCapsGetVersion()     XPR_DevCapsGetInteger(XPR_DEVCAPS_ID_VERSION, XPR_DEVCAPS_DEVID_DEF)
#define XPR_DevCapsGetDateTime()    XPR_DevCapsGetInt64(XPR_DEVCAPS_ID_DATETIME, XPR_DEVCAPS_DEVID_DEF)

#ifdef __cplusplus
}
#endif

#endif // XPR_DEVCAPS_H
