/*
 * File: xpr_onvif.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * ONVIF 协议接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2016-11-25 11:25:25 Friday, 25 November
 * Last Modified : 2019-07-03 04:54:10 Wednesday, 3 July
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
#ifndef XPR_ONVIF_H
#define XPR_ONVIF_H

#include <xpr/xpr_common.h>
#include <xpr/xpr_json.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XPR_ONVIF_PORT(major, minor)    (int)(((int)(major)<<16)|(minor))
#define XPR_ONVIF_PORT_MAJOR(port)      (int)(((port)>>16) & 0xffff)
#define XPR_ONVIF_PORT_MINOR(port)      (int)((port) & 0xffff)
#define XPR_ONVIF_PORT_MAJOR_SVC        0x0001
#define XPR_ONVIF_PORT_MINOR_ANY        0xffff

typedef enum XPR_ONVIF_ConfigType {
    XPR_ONVIF_CFG_MAX_TASKS,
    XPR_ONVIF_CFG_MAX_WORKERS,
} XPR_ONVIF_ConfigType;

typedef void (*XPR_AsyncCallHandler)(void* opaque, XPR_JSON* data);

XPR_API int XPR_ONVIF_Config(int cfg, const void* data, int size);

XPR_API int XPR_ONVIF_Init(void);
XPR_API int XPR_ONVIF_Fini(void);

XPR_API XPR_JSON* XPR_ONVIF_NewData(const char* url, const char* action);
XPR_API void XPR_ONVIF_ReleaseData(XPR_JSON* data);

XPR_API int XPR_ONVIF_DataSetAction(XPR_JSON* data, const char* action);
XPR_API int XPR_ONVIF_DataSetTMO(XPR_JSON* data, int rxTmo, int txTmo);
XPR_API int XPR_ONVIF_DataSetUA(XPR_JSON* data, const char* username,
                                const char* password);
XPR_API int XPR_ONVIF_DataSetURL(XPR_JSON* data, const char* url);
XPR_API int XPR_ONVIF_DataSetXML(XPR_JSON* data, const char* xml, int length);

XPR_API int XPR_ONVIF_AsyncCall(int port, XPR_JSON* data,
                                XPR_AsyncCallHandler cb, void* opaque);
XPR_API int XPR_ONVIF_Call(XPR_JSON* data, XPR_JSON** result);
XPR_API int XPR_ONVIF_ParseRequest(const uint8_t* data, int size,
                                   XPR_JSON** result);

XPR_API int XPR_ONVIF_OpenService(int port, const char* url);
XPR_API int XPR_ONVIF_ServiceAddActionHandler(int port,
                                              XPR_ONVIF_ActionHandler cb,
                                              void* opaque);
XPR_API int XPR_ONVIF_DelActionHandler(int port, XPR_ONVIF_ActionHandler cb,
                                       void* opaque);
XPR_API int XPR_ONVIF_CloseService(int port);

#ifdef __cplusplus
}
#endif

#endif // XPR_ONVIF_H
