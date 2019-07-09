#include <stdio.h>
#include <stdlib.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_plugin.h>
#include <xpr/xpr_sys.h>
#include <xpr/xpr_utils.h>

static int myInit(XPR_PluginRegistry* reg)
{
    DBG(DBG_L3, "%s:%d", __FILE__, __LINE__);
    return XPR_ERR_OK;
}

static int myFini(XPR_PluginRegistry* reg)
{
    DBG(DBG_L3, "%s:%d", __FILE__, __LINE__);
    return XPR_ERR_OK;
}

static const char* myDepends[] = {
    "b",
    NULL,
};

static XPR_PluginRegistry myPlugin = {
    .name = "d",
    .desc = "Plugin example 4",
    .depends = myDepends,
    .version = "1.0.0",
    .init = myInit,
    .fini = myFini,
    .data = 0,
    .dataSize = 0,
};

int XPR_PluginRegister(XPR_PluginRegistry** entries)
{
    *entries = &myPlugin;
    return 1;
}