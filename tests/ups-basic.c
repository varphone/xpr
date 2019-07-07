﻿/*
 * File: ups-basic.c
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *  XPR-UPS 测试用例
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : tests
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2019-07-06 11:20:31 +08:00 Sunday, 06 July
 * Last Modified : 2019-07-07 11:20:47 +08:00 Sunday, 07 July
 * Modified By   : Varphone Wong <varphone@qq.com>
 * ---------------------------------------------------------------------------
 * Copyright (C) 2012 - 2019 CETC55, Technology Development CO.,LTD.
 * Copyright (C) 2010 - 2019 Varphone Wong, Varphone.com
 * All rights reserved.
 * ---------------------------------------------------------------------------
 * HISTORY:
 * 2019-07-07   varphone    初始版本建立
 */
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_ups.h>
#include <xpr/xpr_utils.h>

#define kTestLoops 10000

#if defined(__clang__)
// FIXME:
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-conversion"
#elif defined(_MSC_VER)
// FIXME:
#endif

static void test_XPR_UPS_Init(void)
{
    // Test loop init/fini
    for (int i = 0; i< 10; i++) {
        XPR_UPS_Init("ups-storage.json");
        XPR_UPS_Fini();
    }
}

// Callback for init:/world
XPR_UPS_DEF_INITER(world_init)
{
    return XPR_ERR_OK;
}

// Callback for get:/system/network/*
XPR_USP_DEF_GETTER(network_getter)
{
    printf("Getter %p, %s, %p, %p\n", entry, key, buffer, size);
    // Only address supports
    if (strcmp(key, "address") == 0)
        return XPR_UPS_PibString(buffer, size, "192.168.1.222", -1);
    // Others are not supports
    return XPR_ERR_UPS_NOT_SUPPORT;
}

// Callback for set:/system/network/$perform
XPR_USP_DEF_SETTER(network_perform)
{
    int action = *(int*)(data);
    printf("/system/network/$perform, action=%d\n", action);
    return XPR_ERR_OK;
}

// Define entries: /system
static XPR_UPS_Entry _System[] = {
    // /system
    XPR_UPS_ENTRY_DIR2("system", "Settings for system"),
    // /system/information
    XPR_UPS_ENTRY_DIR4("information", "System informations", NULL, "/system/"),
    XPR_UPS_ENTRY_PAR_STR_DV("firmware", "", "v3.0.0"),
    XPR_UPS_ENTRY_PAR_STR_DV("hardware", "", "v4.0.0"),
    XPR_UPS_ENTRY_PAR_STR_DV("id", "", "SN-00000000"),
    XPR_UPS_ENTRY_PAR_STR_DV("internal_model", "", "HD-25440-20D112000"),
    XPR_UPS_ENTRY_PAR_STR_DV("location", "", "China"),
    XPR_UPS_ENTRY_PAR_STR_DV("manufacturer", "", "Varphone.com"),
    // /system/network
    XPR_UPS_ENTRY_DIR4_G_S("network", "System network settings", "ups/dir",
                           "/system/", network_getter, NULL),
    XPR_UPS_ENTRY_PAR_STR_DV("address", "", "192.168.1.220"),
    XPR_UPS_ENTRY_PAR_I32_S("$perform", "Perform the network setup",
                            network_perform),
};

static void test_XPR_UPS_Case1(void)
{
    if (XPR_UPS_Init("ups-storage.json") != XPR_ERR_OK)
        ;//abort();
    XPR_UPS_Register(_System, _countof(_System));
    XPR_UPS_PrintAll();
    // Bugy test
    XPR_UPS_SetInteger("/", 9988);
    XPR_UPS_SetInteger("/system", 9988);
    // Test set values
    XPR_UPS_SetString("/system/information/firmware", "v3.0.0-new", NULL);
    XPR_UPS_SetString("/system/information/hardware", "v4.0.0-new", NULL);
    XPR_UPS_SetInteger("/system/network/$perform", 0);
    XPR_UPS_SetInteger("/system/network/$perform", 1);
    // Test get/peek values
    char val[64];
    int valSize = sizeof(val);
    XPR_UPS_GetString("/system/network/address", val, &valSize);
    DBG(DBG_L3, "[%d] address = %s", __LINE__, val);
    valSize = sizeof(val);
    XPR_UPS_GetString("/system/network/not-exists", val, &valSize);
    DBG(DBG_L3, "[%d] not-exists = %s", __LINE__, val);
    DBG(DBG_L3, "[%d] address = %s", __LINE__,
        XPR_UPS_PeekString("/system/network/address"));
    XPR_UPS_PrintAll();
    XPR_UPS_Fini();
}

int main(int argc, char** argv)
{
    test_XPR_UPS_Init();
    test_XPR_UPS_Case1();
    return 0;
}