/*
 * File: xpr_plugin.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 插件框架接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2014-11-21 12:50:43 Friday, 21 November
 * Last Modified : 2019-07-03 04:49:36 Wednesday, 3 July
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
#ifndef XPR_PLUGIN_H
#define XPR_PLUGIN_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_PLUGININITFUN_TYPE_DEFINED
#define XPR_PLUGININITFUN_TYPE_DEFINED
///
/// 插件初始化函数
///
typedef void* (*XPR_PluginInitFun)(void);
#endif // XPR_PLUGININITFUN_TYPE_DEFINED

#ifndef XPR_PLUGINFINIFUN_TYPE_DEFINED
#define XPR_PLUGINFINIFUN_TYPE_DEFINED
///
/// 插件释放函数
///
typedef int (*XPR_PluginFiniFun)(void*);
#endif // XPR_PLUGINFINIFUN_TYPE_DEFINED

#ifndef XPR_PLUGIN_TYPE_DEFINED
#define XPR_PLUGIN_TYPE_DEFINED
///
/// 插件数据结构
///
struct XPR_Plugin {
    const char* name; ///< 插件名称
    const char* desc; ///< 插件描述
    void* data; ///< 插件专属数据指针
    unsigned int dataSize; ///< 插件专属数据大小
};
typedef struct XPR_Plugin XPR_Plugin;
#endif // XPR_PLUGIN_TYPE_DEFINED

#ifndef XPR_PLUGINMODULE_TYPE_DEFINED
#define XPR_PLUGINMODULE_TYPE_DEFINED
///
/// 插件模块数据结构
///
struct XPR_PluginModule {
    const char* libName; ///< 插件模块的库名词
    void* libHandle; ///< 插件模块的库句柄
    void* pluginHandle; ///< 插件句柄
    XPR_PluginInitFun init; ///< 插件初始化函数
    XPR_PluginFiniFun fini; ///< 插件释放函数
    struct XPR_PluginModule* prev; ///< 前一个节点
    struct XPR_PluginModule* next; ///< 下一个节点
};
typedef struct XPR_PluginModule XPR_PluginModule;
#endif // XPR_PLUGINMODULE_TYPE_DEFINED

/// @brief Plugin initialize entry
/// @return Plugin handle
XPR_Plugin* XPR_PluginInit(void);

/// @brief Plugin finialize entry
/// @retval 0   success
/// @retval -1  failure
int XPR_PluginFini(XPR_Plugin* plugin);

/// @brief Load single plugin
/// @return Plugin module handle
XPR_PluginModule* XPR_PluginLoad(const char* name);

/// @brief Load all plugins in specified dir
/// @return Number of plugins loaded
int XPR_PluginLoadAll(const char* dir);

/// @brief Unload loaded plugin
/// @retval 0   success
/// @retval -1  failure
int XPR_PluginUnload(XPR_PluginModule* handle);

/// @brief Unload all loaded plugins
/// @return 0   success
/// @return -1  failure
int XPR_PluginUnloadAll(void);

/// @brief Find loaded plugin
/// @param [in] name        Plugin name
/// @return Plugin handle
XPR_Plugin* XPR_PluginFind(const char* name);

/// @biref Get plugin name
const char* XPR_PluginGetName(const XPR_Plugin* plugin);

/// @brief Get plugin description
const char* XPR_PluginGetDesc(const XPR_Plugin* plugin);

/// @brief Get plugin parameter
/// @param [in] plugin      Plugin handle
/// @param [in] param       Parameter id
/// @param [in,out] buffer  Buffer to receive parameter value
/// @param [in,out] size    Buffer size and parameter value size
/// @retval 0   success
/// @retval -1  failure
int XPR_PluginGetParam(XPR_Plugin* plugin, int param, void* buffer, int* size);

/// @biref Set plugin parameter
/// @param [in] plugin      Plugin handle
/// @param [in] param       Parameter id
/// @param [in] data        Data to set
/// @param [in] size        Data size
/// @retval 0   success
/// @retval -1  failure
int XPR_PluginSetParam(XPR_Plugin* plugin, int param, const void* data,
                       int size);

#ifdef __cplusplus
}
#endif

#endif // XPR_PLUGIN_H
