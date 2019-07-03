/*
 * File: xpr_sha1.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * SHA1 哈希接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2016-12-13 09:52:11 Tuesday, 13 December
 * Last Modified : 2019-07-03 04:38:30 Wednesday, 3 July
 * Modified By   : Varphone Wong <varphone@qq.com>
 * ---------------------------------------------------------------------------
 * Copyright (C) Steve Reid <steve@edmweb.com>.
 * Copyright (C) 2012 - 2019 CETC55, Technology Development CO.,LTD.
 * Copyright (C) 2012 - 2019 Varphone Wong, Varphone.com.
 * All rights reserved.
 * ---------------------------------------------------------------------------
 * HISTORY:
 * 2019-07-03   varphone    更新版权信息
 * 2014-11-21   varphone    初始版本建立
 */
#ifndef XPR_SHA1_H
#define XPR_SHA1_H

#include <stdint.h>
#include <xpr/xpr_common.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @brief SHA1 哈希值字串缓冲区长度
#define XPR_SHA1_HASH_BUF	42

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    uint8_t buffer[64];
} XPR_SHA1_CTX;

XPR_API void XPR_SHA1Transform(uint32_t state[5], const uint8_t buffer[64]);

XPR_API void XPR_SHA1Init(XPR_SHA1_CTX* context);

XPR_API void XPR_SHA1Update(XPR_SHA1_CTX* context, const uint8_t *data, size_t length);

XPR_API void XPR_SHA1Final(XPR_SHA1_CTX* context, uint8_t digest[20]);

/// @note `hashOut` must be large than XPR_SHA1_HASH_BUF bytes
XPR_API char* XPR_SHA1Data(const uint8_t* data, size_t length, char* hashOut);

/// @note `hashOut` must be large than XPR_SHA1_HASH_BUF bytes
XPR_API char* XPR_SHA1File(const char* path, char* hashOut);

#ifdef __cplusplus
}
#endif

#endif // XPR_SHA1_H