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

