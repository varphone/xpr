#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <xpr/xpr_plugin.h>

static XPR_PluginModule* pluginRoot = 0;

// FIXME: Thread Safe
static void DropPluginModule(XPR_PluginModule* pm)
{
    if (pluginRoot == pm) {
        pluginRoot = pm->next;
        pluginRoot->prev = 0;
    }
    else {
        pm->prev->next = pm->next;
        if (pm->next)
            pm->next->prev = pm->prev;
    }
}

// FIXME: Thread Safe
static void* FindPluginModule(const char* name)
{
    const char* s = 0;
    XPR_PluginModule* pm = pluginRoot;
    while (pm) {
        if (pm->pluginHandle) {
            s = XPR_PluginGetName(pm->pluginHandle);
            if (s && name && strcmp(s, name) == 0)
                return pm;
        }
        pm = pm->next;
    }
    return 0;
}

// FIXME: Thread Safe
static void HoldPluginModule(XPR_PluginModule* pm)
{
    XPR_PluginModule* p = pluginRoot;
    if (pluginRoot == 0) {
        pluginRoot = pm;
    }
    else {
        while (p) {
            if (p->next == 0) {
                p->next = pm;
                pm->prev = p;
				return;
            }
            p = p->next;
        }
    }
}

XPR_PluginModule* XPR_PluginLoad(const char* name)
{
    XPR_PluginModule* pm = calloc(sizeof(*pm), 1);
    if (pm) {
        pm->libHandle = dlopen(name, RTLD_LAZY);
        if (!pm->libHandle) {
            fprintf(stderr, "%s\n", dlerror());
            goto fail;
        }
        pm->prev = 0;
        pm->next = 0;
        pm->init = dlsym(pm->libHandle, "XPR_PluginInit");
        pm->fini = dlsym(pm->libHandle, "XPR_PluginFini");
        if (pm->init) {
            pm->pluginHandle = pm->init();
            if(pm->pluginHandle ==NULL) {
                fprintf(stderr, "load [%s] failed\n", name);
                goto fail;
            }
        }
        HoldPluginModule(pm);
    }
    return pm;
fail:
    XPR_PluginUnload(pm);
    return 0;
}

static int comp(const void* a, const void* b)
{
    return strcmp(*(char**)a, *(char**)b);
}

int XPR_PluginLoadAll(const char* dir)
{
    int i = 0;
    int n = 0;
    char root[PATH_MAX] = {0};
    char path[PATH_MAX] = {0};
    char* p = 0;
    char* pl[4096] = {0};
    DIR* d = 0;
    struct dirent* de = 0;
    p = realpath(dir ? dir : "./plugins", root);
    if (!p)
        return -1;
    if (access(root, R_OK) < 0)
        return -1;
    d = opendir(root);
    if (!d)
        return -1;
    while ((de = readdir(d))) {
        p = strstr(de->d_name, ".p");
        if (p && p[2] == 0)
            pl[i++] = strdup(de->d_name);
        if (i >= 4096)
            break;
    }
    closedir(d);
    n = i;
    qsort(pl, n, sizeof(pl[0]), comp);
    for (i = 0; i < n; i++) {
        snprintf(path, sizeof(path), "%s/%s", root, pl[i]);
        XPR_PluginLoad(path);
        free(pl[i]);
    }
    return n;
}

int XPR_PluginUnload(XPR_PluginModule* handle)
{
    XPR_PluginModule* pm = (XPR_PluginModule*)handle;
    if (pm) {
        DropPluginModule(pm);
        if (pm->fini) {
            pm->fini(pm->pluginHandle);
            pm->fini = 0;
            pm->pluginHandle = 0;
        }
        if (pm->libHandle) {
            dlclose(pm->libHandle);
            pm->libHandle = 0;
        }
        free((void*)pm);
    }
    return 0;
}

XPR_Plugin* XPR_PluginFind(const char* name)
{
    XPR_PluginModule* pm = FindPluginModule(name);
    return pm ? pm->pluginHandle : 0;
}

const char* XPR_PluginGetName(const XPR_Plugin* plugin)
{
    return plugin ? ((XPR_Plugin*)plugin)->name : 0;
}

const char* XPR_PluginGetDesc(const XPR_Plugin* plugin)
{
    return plugin ? ((XPR_Plugin*)plugin)->desc : 0;
}

int XPR_PluginGetParam(XPR_Plugin* plugin, int param, void* buffer, int* size)
{
    return 0;
}

int XPR_PluginSetParam(XPR_Plugin* plugin, int param, const void* data, int size)
{
    return 0;
}


