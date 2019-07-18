#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <xpr/xpr_atomic.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_file.h>
#include <xpr/xpr_list.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_plugin.h>
#include <xpr/xpr_sync.h>
#include <xpr/xpr_utils.h>

#ifndef PATH_MAX
#define PATH_MAX 512
#endif

struct XPR_PluginModule
{
    XPR_ListNodeSL node;          ///< 链表节点
    XPR_Atomic refcount;          ///< 引用计数
    char* libFileName;            ///< 插件模块的库文件
    void* libHandle;              ///< 插件模块的库句柄
    void* pluginHandle;           ///< 插件句柄
    XPR_PluginModule* parent;     ///< 插件属主
    XPR_PluginRegistry* registry; ///< 插件信息
    int rank;                     ///< 插件排位评分
};

static char sCurrentDir[PATH_MAX] = {0};
static char* sPluginNaming = NULL;
static char* sSerachDirs = NULL;
static XPR_Atomic sHasInited = 0;
static XPR_RecursiveMutex sLock;
static XPR_List* sPluginList = NULL;

#define XPR_PLUGIN_LOCK() XPR_RecursiveMutexLock(&sLock)
#define XPR_PLUGIN_UNLOCK() XPR_RecursiveMutexUnlock(&sLock)

#define MODULE_REF(m) XPR_AtomicInc(&(m)->refcount)
#define MODULE_REFS(m) XPR_AtomicRead(&(m)->refcount)
#define MODULE_UNREF(m) XPR_AtomicDec(&(m)->refcount)

static void* nodeAlloc(void)
{
    XPR_PluginModule* self = XPR_Alloc(sizeof(*self));
    if (self) {
        memset(self, 0, sizeof(*self));
    }
    return self;
}

static void nodeFree(void* ptr)
{
    XPR_Free(ptr);
}

int nodeCompare(void* a, void* b)
{
    XPR_PluginModule* ma = (XPR_PluginModule*)(a);
    XPR_PluginModule* mb = (XPR_PluginModule*)(b);
    if (ma->refcount < mb->refcount)
        return -1;
    if (ma->refcount > mb->refcount)
        return 1;
    return 0;
}

// Return module matched to name in the loaded list
static XPR_PluginModule* findModule(const char* name)
{
    XPR_PluginModule* m = XPR_ListFirstNl(sPluginList);
    while (m) {
        if (m->registry) {
            if (strcmp(name, m->registry->name) == 0)
                break;
        }
        m = XPR_ListNextNl(sPluginList, m);
    }
    return m;
}

struct FindContext {
    const char* name;     // The name of the plugin
    const char* fileName; // The name of file without path
    char* pathBuf;        // The buffer to receive full path
    char* result;         // Set to pathBuf is found
};

static int handleEachFile(void* opaque, const XPR_FileInfo* fileInfo)
{
    struct FindContext* ctx = (struct FindContext*)(opaque);
    if (fileInfo->type == XPR_FILE_TYPE_REG) {
        if (strcmp(ctx->fileName, fileInfo->name) == 0) {
            strcpy(ctx->pathBuf, fileInfo->fullname);
            ctx->result = ctx->pathBuf;
            return XPR_FALSE;
        }
    }
    return XPR_TRUE;
}

static void handleSegment(void* opaque, char* segment)
{
    struct FindContext* ctx = (struct FindContext*)(opaque);
    XPR_FileForEach(segment, handleEachFile, ctx);
}

static char* findFileForName(const char* name, char* pathBuf)
{
    char fileName[256];
    sprintf(fileName, sPluginNaming, name);
    struct FindContext ctx = {name, fileName, pathBuf, NULL};
    if (sCurrentDir[0])
        handleSegment(&ctx, sCurrentDir);
    if (ctx.result == NULL)
        xpr_foreach_s(sSerachDirs, -1, ";", handleSegment, &ctx);
    if (ctx.result == NULL) {
        DBG(DBG_L2, "XPR_PLUGIN: Plugin \"%s\" does not exists!",
            name);
    }
    return ctx.result;
}

// Return XPR_TRUE if the library has loaded
static int isLibraryLoaded(const char* fileName)
{
    XPR_PluginModule* m = XPR_ListFirstNl(sPluginList);
    while (m) {
        if (m->libFileName) {
            if (strcmp(fileName, m->libFileName) == 0)
                return XPR_TRUE;
        }
        m = XPR_ListNextNl(sPluginList, m);
    }
    return XPR_FALSE;
}

// Forwards
static int loadModuleDepends(const char** depends);
static int unloadModule(XPR_PluginModule* m);
static int unloadModuleByName(const char* name);
static int unloadModuleDepends(const char** depends);

static void refModuleRecursive(XPR_PluginModule* m)
{
    if (!m)
        return;
    // Add reference to module
    MODULE_REF(m);
    // Add reference to depends if exists
    if (m->registry) {
        const char** depends = m->registry->depends;
        while (depends && *depends) {
            refModuleRecursive(findModule(*depends));
            depends++;
        }
    }
}

// Load plugin module with file name
static int loadModule(const char* fileName)
{
    DBG(DBG_L5, "XPR_PLUGIN: Loading '%s' ...", fileName);
    if (isLibraryLoaded(fileName)) {
        DBG(DBG_L5, "XPR_PLUGIN: Plugin '%s' alread loaded!", fileName);
        return XPR_ERR_OK;
    }
    void* handle = dlopen(fileName, RTLD_LAZY);
    if (handle == NULL) {
        DBG(DBG_L1, "XPR_PLUGIN: dlopen(%s) failed, errno: %d, reason: %s",
            fileName, errno, dlerror());
        return XPR_ERR_SYS(errno);
    }
    XPR_PluginRegisterFn registerFn = dlsym(handle, "XPR_PluginRegister");
    if (registerFn == NULL) {
        DBG(DBG_L1,
            "XPR_PLUGIN: dlsym(%p, \"XPR_PluginRegister\") failed, errno: %d",
            handle, errno);
        dlclose(handle);
        return XPR_ERR_SYS(errno);
    }
    XPR_PluginRegistry* regs = NULL;
    int numRegs = registerFn(&regs);
    if (numRegs <= 0) {
        dlclose(handle);
        return XPR_ERR_SYS(ENODATA);
    }
    int err = XPR_ERR_OK;
    int loaded = 0;
    XPR_PluginModule* parent = NULL;
    if (numRegs > 1) {
        parent = nodeAlloc();
        parent->node.next = NULL;
        parent->libHandle = handle;
        parent->libFileName = XPR_StrDup(fileName);
        parent->parent = NULL;
        parent->registry = NULL;
        MODULE_REF(parent);
        XPR_ListAppendNl(sPluginList, parent);
    }
    XPR_PluginRegistry* reg = regs;
    for (int i = 0; i< numRegs; i++, reg++) {
        if (!reg->name)
            break;
        // Load all depends if exists
        if (reg->depends)
            err = loadModuleDepends(reg->depends);
        if (err != XPR_ERR_OK)
            break;
        // Invoke initializer
        if (reg->init)
            err = reg->init(reg);
        if (err != XPR_ERR_OK)
            break;
        DBG(DBG_L5, "XPR_PLUGIN: Plugin '%s' initialized!", reg->name);
        // Add module to list
        XPR_PluginModule* m = nodeAlloc();
        if (m == NULL) {
            DBG(DBG_L1, "XPR_PLUGIN: allocate memory failed!");
            // Invoke finalizer
            if (reg->fini)
                reg->fini(reg);
            break;
        }
        m->node.next = NULL;
        m->libHandle = parent ? NULL : handle;
        m->libFileName = parent ? NULL : XPR_StrDup(fileName);
        m->parent = parent;
        m->registry = reg;
        if (parent)
            MODULE_REF(parent);
        MODULE_REF(m);
        XPR_ListAppendNl(sPluginList, m);
        DBG(DBG_L3, "XPR_PLUGIN: Plugin [%s, %s] loaded!", reg->name,
            reg->version);
        // Move to next registry
        loaded++;
    }
    // Cleanup if not success
    if (err != XPR_ERR_OK) {
        reg = regs;
        for (int i = 0; i< loaded; i++, reg++) {
            if (reg->name == NULL)
                break;
            unloadModuleByName(reg->name);
        }
        if (parent && loaded == 0)
            unloadModule(parent);
    }
    return err;
}

// Load all dependency module(s)
static int loadModuleDepends(const char** depends)
{
    if (!depends)
        return XPR_ERR_OK;
    int err = XPR_ERR_OK;
    char path[PATH_MAX];
    XPR_PluginModule* m = NULL;
    while (*depends) {
        DBG(DBG_L5, "XPR_PLUGIN: Loading depends '%s' ...", *depends);
        // Skip if already loaded
        m = findModule(*depends);
        if (m) {
            DBG(DBG_L5, "XPR_PLUGIN: Load depends '%s' ignored!",
                *depends);
            goto next;
        }
        // Find the fullname of the plugin matched to the depends
        if (findFileForName(*depends, path) == NULL) {
            err = XPR_ERR_SYS(ENOENT);
            break;
        }
        // Load now
        err = loadModule(path);
        if (err != XPR_ERR_OK)
            break;
        DBG(DBG_L5, "XPR_PLUGIN: Load depends '%s' okay!", *depends);
        m = findModule(*depends);
next:
        refModuleRecursive(m);
        depends++;
    }
    return err;
}

// Unload the module and remove the list
static int unloadModule(XPR_PluginModule* m)
{
    if (!m)
        return XPR_ERR_GEN_NULL_PTR;
    // Finalize the module befor actual unload
    if (m->registry) {
        if (MODULE_REFS(m) == 1) {
            if (m->registry->fini) {
                int err = m->registry->fini(m->registry);
                if (err != XPR_ERR_OK) {
                    DBG(DBG_L2,
                        "XPR_PLUGIN: Finalize '%s' failed, errno: 0x%08X",
                        m->registry->name, err);
                    return XPR_ERR_GEN_BUSY;
                }
                DBG(DBG_L5, "XPR_PLUGIN: Plugin '%s' finalized!",
                    m->registry->name);
            }
            DBG(DBG_L5, "XPR_PLUGIN: Unloading '%s' @ %p ...",
                m->registry->name, m);
            // Unload all depends
            if (m->registry->depends)
                unloadModuleDepends(m->registry->depends);
        }
    }
    // Unload now if refs == 0
    if (MODULE_UNREF(m) == 0) {
        if (m->libFileName)
            XPR_Free(m->libFileName);
        if (m->libHandle)
            dlclose(m->libHandle);
        // Unload parent if exists
        if (m->parent) {
            if (MODULE_UNREF(m->parent) == 1)
                unloadModule(m->parent);
        }
        XPR_ListRemoveNl(sPluginList, m);
        DBG(DBG_L5, "XPR_PLUGIN: Unload %p okay!", m);
    }
    return XPR_ERR_OK;
}

// Unload module matched to name
static int unloadModuleByName(const char* name)
{
    if (!name)
        return XPR_ERR_GEN_NULL_PTR;
    XPR_PluginModule* m = findModule(name);
    if (!m)
        return XPR_ERR_SYS(ENOENT);
    return unloadModule(m);
}

// Unload modules matched to name list
static int unloadModuleDepends(const char** depends)
{
    XPR_PluginModule* m = NULL;
    while (*depends) {
        m = findModule(*depends);
        if (m) {
            if (MODULE_UNREF(m) == 1)
                unloadModule(m);
        }
        depends++;
    }
    return XPR_ERR_OK;
}

XPR_API int XPR_PluginInit(void)
{
    // Guard for threads safe
    if (XPR_AtomicInc(&sHasInited) == 1) {
        XPR_RecursiveMutexInit(&sLock);
        XPR_PLUGIN_LOCK();
        sPluginNaming = XPR_StrDup("libplugin_%s.so");
        sPluginList = XPR_ListCreate(XPR_LIST_SINGLY_LINKED, nodeAlloc,
                                     nodeFree, nodeCompare);
        XPR_PLUGIN_UNLOCK();
        return XPR_ERR_OK;
    }
    DBG(DBG_L2, "XPR_PLUGIN: Init/Fini counter: %d",
        XPR_AtomicRead(&sHasInited));
    return XPR_ERR_GEN_BUSY;
}

XPR_API int XPR_PluginFini(void)
{
    // Guard for threads safe
    if (XPR_AtomicDec(&sHasInited) == 0) {
        XPR_PLUGIN_LOCK();
        if (sPluginNaming) {
            XPR_Free(sPluginNaming);
            sPluginNaming = NULL;
        }
        if (sSerachDirs) {
            XPR_Free(sSerachDirs);
            sSerachDirs = NULL;
        }
        if (sPluginList) {
            XPR_ListDestroy(sPluginList);
            sPluginList = NULL;
        }
        XPR_PLUGIN_UNLOCK();
        XPR_RecursiveMutexFini(&sLock);
        return XPR_ERR_OK;
    }
    DBG(DBG_L2, "XPR_PLUGIN: Init/Fini counter: %d",
        XPR_AtomicRead(&sHasInited));
    return XPR_ERR_GEN_BUSY;
}

XPR_API void XPR_PluginSetDirs(const char* dirs)
{
    XPR_PLUGIN_LOCK();
    if (sSerachDirs)
        XPR_Free(sSerachDirs);
    sSerachDirs = XPR_StrDup(dirs);
    XPR_PLUGIN_UNLOCK();
}

XPR_API void XPR_PluginSetNaming(const char* fmt)
{
    XPR_PLUGIN_LOCK();
    if (sPluginNaming)
        XPR_Free(sPluginNaming);
    sPluginNaming = XPR_StrDup(fmt);
    XPR_PLUGIN_UNLOCK();
}

XPR_API int XPR_PluginLoad(const char* name)
{
    char path[PATH_MAX] = {0};
    if (realpath(name, path) != NULL) {
        if (access(path, R_OK) < 0)
            return XPR_ERR_SYS(EPERM);
        return loadModule(path);
    }
    if (findFileForName(name, path))
        return loadModule(path);

    return XPR_ERR_SYS(ENOENT);
}

// Callback for load each directory
static void loadEachDir(void* opaque, char* segment)
{
    XPR_PluginLoadDir(segment);
}

XPR_API int XPR_PluginLoadAll(void)
{
    XPR_PLUGIN_LOCK();
    if (sSerachDirs)
        xpr_foreach_s(sSerachDirs, -1, ";", loadEachDir, NULL);
    XPR_PLUGIN_UNLOCK();
    return XPR_ERR_OK;
}

// Callback for XPR_PluginLoadDir ==> XPR_FileForEach
static int loadEachFile(void* opaque, const XPR_FileInfo* fileInfo)
{
    if (fileInfo->type == XPR_FILE_TYPE_REG) {
        if (loadModule(fileInfo->fullname) == XPR_ERR_OK)
                *(int*)(opaque) = *(int*)(opaque) + 1;
    }
    return XPR_TRUE;
}

XPR_API int XPR_PluginLoadDir(const char* dir)
{
    char path[PATH_MAX] = {0};
    if (realpath(dir, path) == NULL)
        return XPR_ERR_SYS(ENOENT);
    if (access(path, R_OK) < 0)
        return XPR_ERR_SYS(EPERM);
    XPR_PLUGIN_LOCK();
    // Cache current directory
    strcpy_s(sCurrentDir, sizeof(sCurrentDir), path);
    // List files in the path
    int loaded = 0;
    XPR_FileForEach(sCurrentDir, loadEachFile, &loaded);
    XPR_PLUGIN_UNLOCK();
    return loaded;
}

XPR_API int XPR_PluginUnload(const char* name)
{
    int err = XPR_ERR_OK;
    XPR_PLUGIN_LOCK();
    err = unloadModuleByName(name);
    XPR_PLUGIN_UNLOCK();
    return err;
}

XPR_API int XPR_PluginUnloadAll(void)
{
    int err = XPR_ERR_OK;
    XPR_PLUGIN_LOCK();
    // Sort all module by refcount (asc)
    XPR_ListSortNl(sPluginList, NULL);
    XPR_PluginModule* m = XPR_ListFirstNl(sPluginList);
    while (m) {
        unloadModule(m);
        // The list is changed, sort again
        XPR_ListSortNl(sPluginList, NULL);
        // Peek next module
        m = XPR_ListFirstNl(sPluginList);
    }
    XPR_PLUGIN_UNLOCK();
    return err;
}

XPR_API void* XPR_PluginFind(const char* name)
{
    XPR_PluginModule* m = NULL;
    XPR_PLUGIN_LOCK();
    m = findModule(name);
    XPR_PLUGIN_UNLOCK();
    return m;
}

XPR_API const char* XPR_PluginGetName(void* plugin)
{
    XPR_PluginModule* m = (XPR_PluginModule*)(plugin);
    if (m->registry)
        return m->registry->name;
    return NULL;
}

XPR_API const char* XPR_PluginGetDesc(void* plugin)
{
    XPR_PluginModule* m = (XPR_PluginModule*)(plugin);
    if (m->registry)
        return m->registry->desc;
    return NULL;
}

XPR_API int XPR_PluginGetParam(void* plugin, int param, void* buffer, int* size)
{
    XPR_PluginModule* m = (XPR_PluginModule*)(plugin);
    if (m->registry && m->registry->getParam)
        return m->registry->getParam(m->registry, param, buffer, size);
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_PluginSetParam(void* plugin, int param, const void* data,
                               int size)
{
    XPR_PluginModule* m = (XPR_PluginModule*)(plugin);
    if (m->registry && m->registry->setParam)
        return m->registry->setParam(m->registry, param, data, size);
    return XPR_ERR_GEN_NOT_SUPPORT;
}
