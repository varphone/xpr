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
    const char* libFileName;      ///< 插件模块的库文件
    void* libHandle;              ///< 插件模块的库句柄
    void* pluginHandle;           ///< 插件句柄
    XPR_PluginModule* parent;     ///< 插件属主
    XPR_PluginRegistry* registry; ///< 插件信息
    int numRegistries;            ///< 插件数量
    int rank;                     ///< 插件排位评分
};

static char sCurrentDir[PATH_MAX] = {0};
static char* sSerachDirs = NULL;
static XPR_Atomic sHasInited = 0;
static XPR_RecursiveMutex sLock;
static XPR_List* sPluginList = NULL;

#define XPR_PLUGIN_LOCK() XPR_RecursiveMutexLock(&sLock)
#define XPR_PLUGIN_UNLOCK() XPR_RecursiveMutexUnlock(&sLock)

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
    if (ma->rank < mb->rank)
        return -1;
    if (ma->rank > mb->rank)
        return 1;
    return 0;
}

// Return module matched to name in the loaded list
static XPR_PluginModule* findModule(const char* name)
{
    XPR_PluginModule* m = XPR_ListFirstNl(sPluginList);
    while (m) {
        if (strcmp(name, m->registry->name) == 0)
            break;
        m = XPR_ListNextNl(sPluginList, m);
    }
    return m;
}

struct FindContext {
    const char* name;
    char* pathBuf;
    char* result;
};

static void handleSegment(void* opaque, char* segment)
{
    struct FindContext* ctx = (struct FindContext*)(opaque);
    char basename[256];
    char* result = NULL;
    sprintf(basename, "plugin_%s.p", ctx->name);
    char** list = NULL;
    int numFiles = 0;
    numFiles = XPR_FilesInDir(segment, &list);
    if (numFiles > 0) {
        for (int i = 0; i < numFiles; i++) {
            if (strstr(list[i], basename)) {
                strcpy(ctx->pathBuf, list[i]);
                ctx->result = ctx->pathBuf;
            }
        }
        XPR_Freev((void**)list);
    }
}

static char* findFileForName(const char* name, char* pathBuf)
{
    struct FindContext ctx = {name, pathBuf, NULL};
    if (sCurrentDir[0])
        handleSegment(&ctx, sCurrentDir);
    if (ctx.result == NULL)
        xpr_foreach_s(sSerachDirs, -1, ";", handleSegment, &ctx);
    if (ctx.result == NULL) {
        DBG(DBG_L2, "XPR_PLUGIN: The file of plugin \"%s\" does not exists",
            name);
    }
    return ctx.result;
}

// Forwards
static int loadModuleDepends(const char** depends);

// Load plugin module with file name
static int loadModule(const char* fileName)
{
    void* handle = dlopen(fileName, RTLD_LAZY);
    if (handle == NULL) {
        DBG(DBG_L1, "XPR_PLUGIN: dlopen(%s) failed, errno: %d", fileName,
            errno);
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
    XPR_PluginModule* parent = NULL;
    if (numRegs > 1) {
        parent = nodeAlloc();
        parent->node.next = NULL;
        parent->libHandle = handle;
        parent->libFileName = fileName;
        parent->parent = NULL;
        parent->registry = regs;
        parent->numRegistries = numRegs;
        XPR_ListAppendNl(sPluginList, parent);
    }
    for (int i = 0; i< numRegs; i++, regs++) {
        if (regs->name == NULL)
            break;
        if (regs->depends)
            err = loadModuleDepends(regs->depends);
        if (regs->init)
            err = regs->init(regs);
        if (err == XPR_ERR_OK) {
            XPR_PluginModule* m = nodeAlloc();
            if (m == NULL) {
                DBG(DBG_L1, "XPR_PLUGIN: allocate memory failed！");
                dlclose(handle);
                return XPR_ERR_SYS(ENOMEM);
            }
            m->node.next = NULL;
            m->libHandle = parent ? NULL : handle;
            m->libFileName = parent ? NULL : fileName;
            m->parent = parent;
            m->registry = regs;
            XPR_ListAppendNl(sPluginList, m);
            DBG(DBG_L3, "XPR_PLUGIN: Plugin [%s, %s] loaded!", regs->name,
                regs->version);
        }
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
    while (*depends) {
        // Skip if already loaded
        if (findModule(*depends) != NULL)
            goto next;
        if (findFileForName(*depends, path) == NULL) {
            err = XPR_ERR_SYS(ENOENT);
            break;
        }
        // Load now
        err = loadModule(path);
        if (err != XPR_ERR_OK)
            break;
next:
        depends++;
    }
    return err;
}

XPR_API int XPR_PluginInit(void)
{
    // Guard for threads safe
    if (XPR_AtomicInc(&sHasInited) == 1) {
        XPR_RecursiveMutexInit(&sLock);
        XPR_PLUGIN_LOCK();
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
        if (sSerachDirs)
            XPR_Free(sSerachDirs);
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

static int comp(const void* a, const void* b)
{
    return strcmp(*(char**)a, *(char**)b);
}

XPR_API int XPR_PluginLoadAll(void)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_PluginLoadDir(const char* dir)
{
    char path[PATH_MAX] = {0};
    if (realpath(dir, path) == NULL)
        return XPR_ERR_SYS(ENOENT);
    if (access(path, R_OK) < 0)
        return XPR_ERR_SYS(EPERM);
    // Cache current directory
    strcpy_s(sCurrentDir, sizeof(sCurrentDir), path);
    // List files in the path
    char** files = NULL;
    int numLoaded = 0;
    int numFiles = XPR_FilesInDir(path, &files);
    if (numFiles > 0) {
        // Sort first
        qsort(files, numFiles, sizeof(char*), comp);
        for (int i = 0; i < numFiles; i++) {
            if (!xpr_ends_with(files[i], ".p"))
                continue;
            if (loadModule(files[i]) == XPR_ERR_OK)
                numLoaded++;
        }
    }
    XPR_Freev((void**)(files));
    return numLoaded;
}

XPR_API int XPR_PluginUnload(const char* name)
{
    // FIXME:
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API void* XPR_PluginFind(const char* name)
{
    // FIXME:
    return NULL;
}

XPR_API const char* XPR_PluginGetName(void* plugin)
{
    // FIXME:
    return NULL;
}

XPR_API const char* XPR_PluginGetDesc(void* plugin)
{
    // FIXME:
    return NULL;
}

XPR_API int XPR_PluginGetParam(void* plugin, int param, void* buffer, int* size)
{
    // FIXME:
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_PluginSetParam(void* plugin, int param, const void* data,
                               int size)
{
    // FIXME:
    return XPR_ERR_GEN_NOT_SUPPORT;
}
