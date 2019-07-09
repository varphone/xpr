#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <xpr/xpr_plugin.h>

#if defined(DEBUG) && defined(DEBUG_POSTFIX)
#define NAMING "libplugin_%s" DEBUG_POSTFIX ".so"
#else
#define NAMING "libplugin_%s.so"
#endif

int main(int argc, char** argv)
{
    XPR_PluginInit();
#if 1
    XPR_PluginSetDirs("plugins;/usr/loca/plugins");
    XPR_PluginSetNaming(NAMING);
    XPR_PluginLoad("c");
    XPR_PluginLoadDir("plugins");
    XPR_PluginLoadAll();
#else
    XPR_PluginLoadDir("plugins");
#endif
    XPR_PluginUnloadAll();
    XPR_PluginFini();
    return 0;
}

