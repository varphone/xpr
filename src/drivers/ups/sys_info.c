#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <xpr/xpr_ups.h>
#include <xpr/xpr_utils.h>


static int information_common_get(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key, void* buffer, int* size)
{
    return XPR_UPS_ReadData(ent, json, key, buffer, size);
}

static int information_common_set(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key, const void* data, int size)
{	
    if(-1 == XPR_UPS_WriteData(ent, json, key, data, size))
        return XPR_ERR_UPS_WRITE;

    return XPR_ERR_OK;
}

/*
        /system/information/
*/
static const char* xpr_ups_driver_sys_names[] = { "system", 0};
static const char* xpr_ups_driver_sys_descs[] = { "system", 0};

static const char* xpr_ups_driver_sys_info_names[] = { "information", 0};
static const char* xpr_ups_driver_sys_info_descs[] = { "information", 0};

static const char* xpr_ups_driver_sys_info_strings_names[] = { "name", "location", "manufacturer", "model", "serial_number", "internal_model", "hardware", "software", "firmware", "onvif_version", "onvif_url", "uuid", "id", 0};
static const char* xpr_ups_driver_sys_info_strings_descs[] = { "name", "location", "manufacturer", "model", "serial_number", "internal_model", "hardware", "software", "firmware", "onvif_version", "onvif_url", "uuid", "id", 0};

static const char* xpr_ups_driver_sys_time_names[] = { "time", 0};
static const char* xpr_ups_driver_sys_time_descs[] = { "time", 0};

static const char* xpr_ups_driver_sys_time_strings_names[] = { "time_zone", "date_time", 0};
static const char* xpr_ups_driver_sys_time_strings_descs[] = { "time_zone", "date_time", 0};

XPR_UPS_Entry xpr_ups_driver_system_information[] = {

    {
        xpr_ups_driver_sys_names,
        xpr_ups_driver_sys_descs,
        "ups/dir",
        "/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },

    {
        xpr_ups_driver_sys_info_names,
        xpr_ups_driver_sys_info_descs,
        "ups/dir",
        "/system/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },

    {	
        xpr_ups_driver_sys_info_strings_names,
        xpr_ups_driver_sys_info_strings_descs,
        "ups/entry",
        "/system/information/",
        XPR_UPS_ENTRY_TYPE_STRING,
        0, 0, information_common_get, information_common_set,
        0, 0, 0
    },
    {
        xpr_ups_driver_sys_time_names,
        xpr_ups_driver_sys_time_descs,
        "ups/entry",
        "/system/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_sys_time_strings_names,
        xpr_ups_driver_sys_time_strings_descs,
        "ups/entry",
        "/system/time/",
        XPR_UPS_ENTRY_TYPE_STRING,
        0, 0, information_common_get, information_common_set,
        0, 0, 0
    },

};

const int xpr_ups_driver_system_information_count =  _countof(xpr_ups_driver_system_information);
