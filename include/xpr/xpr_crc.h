/*
 * File: xpr_crc.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * CRC16/32 操作接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2016-11-25 03:04:00 Friday, 25 November
 * Last Modified : 2019-07-03 05:25:16 Wednesday, 3 July
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
#ifndef XPR_CRC_H
#define XPR_CRC_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <xpr/xpr_common.h>

#ifdef __cplusplus
extern "C" {
#endif

// 计算 16 位 CRC 值
XPR_API uint16_t XPR_CRC16(uint16_t crc, const void* buf, size_t size);

// 计算 32 位 CRC 值
XPR_API uint32_t XPR_CRC32(uint32_t crc, const void* buf, size_t size);

#ifdef __cplusplus
}
#endif

#endif // XPR_CRC_H
