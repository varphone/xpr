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
#else
    XPR_PluginLoadDir("plugins");
#endif
    XPR_PluginFini();
    return 0;
}

