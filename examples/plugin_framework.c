#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <xpr/xpr_plugin.h>

int main(int argc, char** argv)
{
    XPR_PluginInit();
#if 1
    XPR_PluginSetDirs("plugins;/usr/loca/plugins");
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

