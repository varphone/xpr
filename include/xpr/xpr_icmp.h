/*
 * File: xpr_icmp.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * ICMP 操作接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2016-11-25 11:25:25 Friday, 25 November
 * Last Modified : 2019-07-03 05:11:49 Wednesday, 3 July
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
#ifndef XPR_ICMP_H
#define XPR_ICMP_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_ICMP_TYPE_DEFINED
#define XPR_ICMP_TYPE_DEFINED
struct XPR_ICMP;
typedef struct XPR_ICMP XPR_ICMP;
#endif // XPR_ICMP_TYPE_DEFINED

XPR_ICMP* XPR_ICMP_New(const char* dev);
int XPR_ICMP_Destroy(XPR_ICMP* arp);
int XPR_ICMP_Ping(XPR_ICMP* arp, const char* host);

#ifdef __cplusplus
}
#endif

#endif // XPR_ICMP_H
