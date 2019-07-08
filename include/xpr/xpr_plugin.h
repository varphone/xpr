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

#include <xpr/xpr_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_PLUGINREGISTRY_TYPE_DEFINED
#define XPR_PLUGINREGISTRY_TYPE_DEFINED
struct XPR_PluginRegistry;
typedef struct XPR_PluginRegistry XPR_PluginRegistry;
#endif // XPR_PLUGINREGISTRY_TYPE_DEFINED

#ifndef XPR_PLUGINREGISTERFN_TYPE_DEFINED
#define XPR_PLUGINREGISTERFN_TYPE_DEFINED
/// 插件注册函数
typedef int (*XPR_PluginRegisterFn)(XPR_PluginRegistry** entries);
#endif // XPR_PLUGINREGISTERFN_TYPE_DEFINED

#ifndef XPR_PLUGININITFN_TYPE_DEFINED
#define XPR_PLUGININITFN_TYPE_DEFINED
/// 插件初始化函数
typedef int (*XPR_PluginInitFn)(XPR_PluginRegistry*);
#endif // XPR_PLUGININITFN_TYPE_DEFINED

#ifndef XPR_PLUGINFINIFN_TYPE_DEFINED
#define XPR_PLUGINFINIFN_TYPE_DEFINED
/// 插件释放函数
typedef int (*XPR_PluginFiniFn)(XPR_PluginRegistry*);
#endif // XPR_PLUGINFINIFN_TYPE_DEFINED

#ifdef XPR_PLUGINREGISTRY_TYPE_DEFINED
/// 插件注册信息
struct XPR_PluginRegistry {
    const char* name;      ///< 插件名称
    const char* desc;      ///< 插件描述
    const char** depends;  ///< 插件依赖列表
    const char* version;   ///< 插件版号
    XPR_PluginInitFn init; ///< 插件初始化函数
    XPR_PluginFiniFn fini; ///< 插件释放函数
    void* data;            ///< 插件专属数据指针
    unsigned int dataSize; ///< 插件专属数据大小
};
#endif // XPR_PLUGINREGISTRY_TYPE_DEFINED

#ifndef XPR_PLUGINMODULE_TYPE_DEFINED
#define XPR_PLUGINMODULE_TYPE_DEFINED
/// 插件模块定义
struct XPR_PluginModule;
typedef struct XPR_PluginModule XPR_PluginModule;
#endif // XPR_PLUGINMODULE_TYPE_DEFINED

/// Initialize the plugin framework
/// @retval XPE_ERR_OK  Success
/// @retval Others      Error
XPR_API int XPR_PluginInit(void);

/// Finalize the plugin framework
/// @retval XPE_ERR_OK  Success
/// @retval Others      Error
XPR_API int XPR_PluginFini(void);

/// Set default search directories
/// @note Support multiple directories with semicolon seperated string
/// @example: "/path/to/my/plugins;/usr/local/plugins"
XPR_API void XPR_PluginSetDirs(const char* dirs);

/// Load single plugin by name of file name
/// @retval XPE_ERR_OK  Success
/// @retval Others      Error
XPR_API int XPR_PluginLoad(const char* name);

/// Load all plugins in specified dirs
/// @retval XPE_ERR_OK  Success
/// @retval Others      Error
XPR_API int XPR_PluginLoadAll(void);

/// Load some plugins in specified dir
/// @retval XPE_ERR_OK  Success
/// @retval Others      Error
XPR_API int XPR_PluginLoadDir(const char* dir);

/// Unload loaded plugin matched to name
/// @retval XPE_ERR_OK  Success
/// @retval Others      Error
XPR_API int XPR_PluginUnload(const char* name);

/// Unload all loaded plugins
/// @retval XPE_ERR_OK  Success
/// @retval Others      Error
XPR_API int XPR_PluginUnloadAll(void);

/// Return loaded plugin matched to name
/// @param [in] name        Plugin name of file name
/// @return Plugin handle or NULL
XPR_API void* XPR_PluginFind(const char* name);

/// Return the name of the plugin
XPR_API const char* XPR_PluginGetName(void* plugin);

/// Return the description of the plugin
XPR_API const char* XPR_PluginGetDesc(void* plugin);

/// Get plugin parameter
/// @param [in] plugin      Plugin handle
/// @param [in] param       Parameter id
/// @param [in,out] buffer  Buffer to receive parameter value
/// @param [in,out] size    Buffer size and parameter value size
/// @retval XPE_ERR_OK  Success
/// @retval Others      Error
XPR_API int XPR_PluginGetParam(void* plugin, int param, void* buffer,
                               int* size);

/// Set plugin parameter
/// @param [in] plugin      Plugin handle
/// @param [in] param       Parameter id
/// @param [in] data        Data to set
/// @param [in] size        Data size
/// @retval XPE_ERR_OK  Success
/// @retval Others      Error
XPR_API int XPR_PluginSetParam(void* plugin, int param, const void* data,
                               int size);

#ifdef __cplusplus
}
#endif

#endif // XPR_PLUGIN_H
