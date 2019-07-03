/*
 * File: xpr_meta.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 通用元数据接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2019-04-22 08:13:22 Monday, 22 April
 * Last Modified : 2019-07-03 04:54:59 Wednesday, 3 July
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
#ifndef XPR_META_H
#define XPR_META_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// 前置声明
/// 音频元数据的结构体
struct XPR_AudioMeta;
typedef struct XPR_AudioMeta XPR_AudioMeta;

struct XPR_AudioMeta {
    uint32_t bitrate;
    uint8_t bitsPerSample;
    uint8_t channelConfig;
    uint8_t numOfChannels;
    uint8_t profile;
    uint32_t samplingFrequency;
    uint8_t sampleRateIndex;
};

#ifdef __cplusplus
}
#endif

#endif // XPR_META_H
