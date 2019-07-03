/*
 * File: xpr_base64.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * BASE64 操作接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2014-11-21 12:50:43 Friday, 21 November
 * Last Modified : 2019-07-03 05:28:07 Wednesday, 3 July
 * Modified By   : Varphone Wong <varphone@qq.com>
 * ---------------------------------------------------------------------------
 * Copyright (C) 1996 - 2014 Live Networks, Inc.
 * Copyright (C) 2012 - 2019 CETC55, Technology Development CO.,LTD.
 * Copyright (C) 2012 - 2019 Varphone Wong, Varphone.com.
 * All rights reserved.
 * ---------------------------------------------------------------------------
 * HISTORY:
 * 2019-07-03   varphone    更新版权信息
 * 2014-11-21   varphone    初始版本建立
 */
#ifndef XPR_BASE64_H
#define XPR_BASE64_H

#include <xpr/xpr_common.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @brief returns a newly allocated array - of size "resultSize"
///        that the caller is responsible for delete[]ing.
///        As above, but includes the size of the input string
///        (i.e., the number of bytes to decode) as a parameter.
///        This saves an extra call to "strlen()" if we already know
///        the length of the input string.
XPR_API unsigned char* XPR_Base64Decode(const char* in, unsigned int inSize,
                                        unsigned int* resultSize,
                                        int trimTrailingZeros);

/// @brief returns a 0-terminated string
///        that the caller is responsible for delete[]ing.
XPR_API char* XPR_Base64Encode(const unsigned char* orig,
                               unsigned int origLength);

#ifdef __cplusplus
}
#endif

#endif
