#include <stdio.h>
#include <stdlib.h>
#include <xpr/xpr_plugin.h>
#include <xpr/xpr_sys.h>

static XPR_Plugin plugin = {
    "ex1",
    "Plugin example 1",
    0,
    0,
};

XPR_Plugin* XPR_PluginInit(void)
{
    XPR_SYS_Reboot();
    return &plugin;
}

int XPR_PluginFini(XPR_Plugin* plugin)
{
    XPR_SYS_Poweroff();
    return 0;
}

