/*
 * File: xpr_arp.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * ARP 操作接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2016-11-25 11:25:24 Friday, 25 November
 * Last Modified : 2019-07-03 05:33:05 Wednesday, 3 July
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
#ifndef XPR_ARP_H
#define XPR_ARP_H

#include <xpr/xpr_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_ARP_TYPE_DEFINED
#define XPR_ARP_TYPE_DEFINED
struct XPR_ARP;
typedef struct XPR_ARP XPR_ARP;
#endif // XPR_ARP_TYPE_DEFINED

XPR_API XPR_ARP* XPR_ARP_New(const char* dev);

XPR_API int XPR_ARP_Destroy(XPR_ARP* arp);

XPR_API int XPR_ARP_Scan(XPR_ARP* arp, const char* host);

#ifdef __cplusplus
}
#endif

#endif // XPR_ARP_H
