#ifndef XPR_PLUGIN_H
#define XPR_PLUGIN_H

#ifdef __cplusplus
extern "C" {
#endif

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
int XPR_PluginSetParam(XPR_Plugin* plugin, int param, const void* data, int size);

#ifdef __cplusplus
}
#endif

#endif // XPR_PLUGIN_H

