#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <xpr/xpr_ups.h>
#include <xpr/xpr_utils.h>

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wint-conversion"
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-conversion"
#elif defined(_MSC_VER)
// FIXME:
#endif

// Callback for get:/system/information/*
XPR_UPS_DEF_GETTER(info_getter)
{
    // FIXME:
    // Return ERROR to invoke default getter
    return XPR_ERR_UPS_SYS_NOTREADY;
}

// Callback for set:/system/information/*
XPR_UPS_DEF_SETTER(info_setter)
{
    // FIXME:
    // Return OK to accept the value
    return XPR_ERR_OK;
}

// Callback for get:/system/time/*
XPR_UPS_DEF_GETTER(time_getter)
{
    // Return ERROR to invoke default getter
    return XPR_ERR_UPS_SYS_NOTREADY;
}

// Callback for set:/system/time/*
XPR_UPS_DEF_SETTER(time_setter)
{
    // Return OK to accept the value
    return XPR_ERR_OK;
}

// Default value defines
#define DV_NAME "Unknown"
#define DV_LOCATION "China"
#define DV_MANUFACTUER "Unknown"
#define DV_MODEL "Unknown"
#define DV_SERIAL_NUMBER "SN-00000000"
#define DV_INTERNAL_MODEL "Unknown"
#define DV_HARDWARE "Unknown"
#define DV_SOFTWARE "Unknown"
#define DV_FIRMWARE "Unknown"
#define DV_ONVIF_VERSION "Unknown"
#define DV_ONVIF_URL "Unknown"
#define DV_UUID "BD4780B6-3136-4117-B365-000003BF3ACA"
#define DV_ID "Unknown"
#define DV_DATE_TIME "2019-07-07T12:00:00"
#define DV_TIME_ZONE "UTC+08:00"

XPR_UPS_Entry __xpr_ups_driver_sys_info[] = {
    XPR_UPS_ENTRY_DIR4_G_S("information", "System informations", NULL,
                           "/system/", info_getter, info_setter),
    XPR_UPS_ENTRY_PAR_STR_DV("name", "", DV_NAME),
    XPR_UPS_ENTRY_PAR_STR_DV("location", "", DV_LOCATION),
    XPR_UPS_ENTRY_PAR_STR_DV("manufacturer", "", DV_MANUFACTUER),
    XPR_UPS_ENTRY_PAR_STR_DV("model", "", DV_MODEL),
    XPR_UPS_ENTRY_PAR_STR_DV("serial_number", "", DV_SERIAL_NUMBER),
    XPR_UPS_ENTRY_PAR_STR_DV("internal_model", "", DV_INTERNAL_MODEL),
    XPR_UPS_ENTRY_PAR_STR_DV("hardware", "", DV_HARDWARE),
    XPR_UPS_ENTRY_PAR_STR_DV("software", "", DV_SOFTWARE),
    XPR_UPS_ENTRY_PAR_STR_DV("firmware", "", DV_FIRMWARE),
    XPR_UPS_ENTRY_PAR_STR_DV("onvif_version", "", DV_ONVIF_VERSION),
    XPR_UPS_ENTRY_PAR_STR_DV("onvif_url", "", DV_ONVIF_URL),
    XPR_UPS_ENTRY_PAR_STR_DV("uuid", "", DV_UUID),
    XPR_UPS_ENTRY_PAR_STR_DV("id", "", DV_ID),
    XPR_UPS_ENTRY_DIR4_G_S("time", "System Date Time", NULL, "/system/",
                           time_getter, time_setter),
    XPR_UPS_ENTRY_PAR_STR_DV("date_time", "", DV_DATE_TIME),
    XPR_UPS_ENTRY_PAR_STR_DV("time_zone", "", DV_TIME_ZONE),
    XPR_UPS_ENTRY_END(),
};

const int __xpr_ups_driver_sys_info_count =  _countof(__xpr_ups_driver_sys_info);

#if defined(__clang__)
#pragma clang diagnostic pop
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic pop
#elif defined(_MSC_VER)
// FIXME:
#endif
