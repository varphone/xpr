/*
 * File: xpr_pluginpriv.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 插件框架接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2014-11-21 12:50:43 Friday, 21 November
 * Last Modified : 2019-07-03 04:47:03 Wednesday, 3 July
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
#ifndef XPR_PLUGINPRIV_H
#define XPR_PLUGINPRIV_H

#ifndef XPR_PLUGININITFUN_TYPE_DEFINED
#define XPR_PLUGININITFUN_TYPE_DEFINED
typedef void* (*XPR_PluginInitFun)(void);
#endif // XPR_PLUGININITFUN_TYPE_DEFINED

#ifndef XPR_PLUGINFINIFUN_TYPE_DEFINED
#define XPR_PLUGINFINIFUN_TYPE_DEFINED
typedef int (*XPR_PluginFiniFun)(void*);
#endif // XPR_PLUGINFINIFUN_TYPE_DEFINED

#ifndef XPR_PLUGIN_TYPE_DEFINED
#define XPR_PLUGIN_TYPE_DEFINED
struct XPR_Plugin {
    const char* name;
    const char* desc;
    void* data;
    unsigned int dataSize;
};
typedef struct XPR_Plugin XPR_Plugin;
#endif // XPR_PLUGIN_TYPE_DEFINED

#ifndef XPR_PLUGINMODULE_TYPE_DEFINED
#define XPR_PLUGINMODULE_TYPE_DEFINED
struct XPR_PluginModule {
    const char* libName;
    void* libHandle;
    void* pluginHandle;
    XPR_PluginInitFun init;
    XPR_PluginFiniFun fini;
    struct XPR_PluginModule* prev;
    struct XPR_PluginModule* next;
};
typedef struct XPR_PluginModule XPR_PluginModule;
#endif // XPR_PLUGINMODULE_TYPE_DEFINED

#endif // XPR_PLUGINPRIV_H
