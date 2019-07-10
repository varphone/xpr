#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_ups.h>
#include <xpr/xpr_utils.h>

#if defined(__clang__)
// FIXME:
#elif defined(__GNUC__) || defined(__GNUG__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-conversion"
#elif defined(_MSC_VER)
// FIXME:
#endif

// Callback for get:/system/network/*
XPR_USP_DEF_GETTER(net_getter)
{
    // FIXME:
    return XPR_ERR_UPS_SYS_NOTREADY;
}

// Callback for set:/system/network/*
XPR_USP_DEF_SETTER(net_setter)
{
    // FIXME:
    return XPR_ERR_UPS_SYS_NOTREADY;
}

// Callback for get:/system/network/eth0|1/*
XPR_USP_DEF_GETTER(eth_getter)
{
    return XPR_ERR_UPS_SYS_NOTREADY;
}

// Callback for set:/system/network/eth0|1/*
XPR_USP_DEF_SETTER(eth_setter)
{
    const char* name = entry->name;
    if (strcmp(name, "mac") == 0) {
        char cmd[1024];
        int result = 0;
        snprintf(cmd, sizeof(cmd), "ifconfig eth1 hw ether %s", (char*)data);
        result = system("ifconfig eth1 down");
        result = system(cmd);
        result = system("ifconfig eth1 up");
        return result == 0 ? XPR_ERR_OK : XPR_ERR_SYS(errno);
    }

    return XPR_ERR_UPS_SYS_NOTREADY;
}

// Callback for get:/system/network/eth0|1/ipv4/*
XPR_USP_DEF_GETTER(ipv4_getter)
{
    return XPR_ERR_UPS_SYS_NOTREADY;
}

// Callback for set:/system/network/eth0|1/ipv4/*
XPR_USP_DEF_SETTER(ipv4_setter)
{
    int err = 0;
    int ipv4[4];
    char cmd[1024];
    const char* name = entry->name;
    if (strcmp(name, "address") == 0) {
        if (sscanf(data, "%d.%d.%d.%d", &ipv4[0], &ipv4[1], &ipv4[2],
                   &ipv4[3]) != 4)
            return XPR_ERR_UPS_ILLEGAL_PARAM;
        snprintf(cmd, sizeof(cmd), "ifconfig eth1 %s", (char*)data);
        err = system(cmd);
    }
    else if (strcmp(name, "netmask") == 0) {
        if (sscanf(data, "%d.%d.%d.%d", &ipv4[0], &ipv4[1], &ipv4[2],
                   &ipv4[3]) != 4)
            return XPR_ERR_UPS_ILLEGAL_PARAM;
        snprintf(cmd, sizeof(cmd), "ifconfig eth1 netmask %s", (char*)data);
        err = system(cmd);
    }
    else if (strcmp(name, "gateway") == 0) {
        if (sscanf(data, "%d.%d.%d.%d", &ipv4[0], &ipv4[1], &ipv4[2],
                   &ipv4[3]) != 4)
            return XPR_ERR_UPS_ILLEGAL_PARAM;
        snprintf(cmd, sizeof(cmd), "route add default gw %s", (char*)data);
        err = system(cmd);
    }

    if (err < 0)
        return XPR_ERR_UPS_NOT_SUPPORT;

    return XPR_ERR_UPS_SYS_NOTREADY;
}

// Callback for get:/system/network/eth0|1/ipv6/*
XPR_USP_DEF_GETTER(ipv6_getter)
{
    return XPR_ERR_UPS_SYS_NOTREADY;
}

// Callback for set:/system/network/eth0|1/ipv6/*
XPR_USP_DEF_SETTER(ipv6_setter)
{
    return XPR_ERR_UPS_SYS_NOTREADY;
}

// Default value defines
#define DV_ETH0_MAC "11:22:33:44:55:66"
#define DV_ETH0_IPV4_ADDRESS "192.168.1.88"
#define DV_ETH0_IPV4_NETMASK "255.255.255.0"
#define DV_ETH0_IPV4_GATEWAY "192.168.1.1"
#define DV_ETH0_IPV4_DNS1 "192.168.1.1"
#define DV_ETH0_IPV4_DNS2 "8.8.8.8"
#define DV_ETH1_MAC "11:22:33:44:55:66"
#define DV_ETH1_IPV4_ADDRESS "192.168.1.88"
#define DV_ETH1_IPV4_NETMASK "255.255.255.0"
#define DV_ETH1_IPV4_GATEWAY "192.168.1.1"
#define DV_ETH1_IPV4_DNS1 "192.168.1.1"
#define DV_ETH1_IPV4_DNS2 "8.8.8.8"
#define DV_HTTP_ENABLED 1
#define DV_HTTP_PORT 80
#define DV_HTTPS_ENABLED 1
#define DV_HTTPS_PORT 443
#define DV_NTP_ENABLED 1
#define DV_NTP_SERVER_ADDRESS "0.japan.pool.ntp.org"
#define DV_NTP_SERVER_PORT 443
#define DV_RTSP_ENABLED 1
#define DV_RTSP_PORT 554

XPR_UPS_Entry __xpr_ups_driver_sys_net[] = {
    // /system/network
    XPR_UPS_ENTRY_DIR4_G_S("network", "System network settings", "ups/dir",
                           "/system/", net_getter, net_setter),
    // /system/network/eth0
    XPR_UPS_ENTRY_DIR4_G_S("eth0", "Ethernet0", "ups/dir", NULL, eth_getter,
                           eth_setter),
    XPR_UPS_ENTRY_PAR_STR_DV("mac", "", DV_ETH0_MAC),
    // /system/network/eth0/ipv4
    XPR_UPS_ENTRY_DIR4_G_S("ipv4", "IPv4", "ups/dir", NULL, ipv4_getter,
                           ipv4_setter),
    XPR_UPS_ENTRY_PAR_STR_DV("address", "", DV_ETH0_IPV4_ADDRESS),
    XPR_UPS_ENTRY_PAR_STR_DV("netmask", "", DV_ETH0_IPV4_NETMASK),
    XPR_UPS_ENTRY_PAR_STR_DV("gateway", "", DV_ETH0_IPV4_GATEWAY),
    XPR_UPS_ENTRY_PAR_STR_DV("dns1", "", DV_ETH0_IPV4_DNS1),
    XPR_UPS_ENTRY_PAR_STR_DV("dns2", "", DV_ETH0_IPV4_DNS2),
    XPR_UPS_ENTRY_PAR_I32("$perform", "Perform IPv4 Changes"),
    // /system/network/eth0/ipv6
    XPR_UPS_ENTRY_DIR4_G_S("ipv6", "IPv6", "ups/dir", "/system/network/eth0",
                           ipv6_getter, ipv6_setter),
    // /system/network/eth1
    XPR_UPS_ENTRY_DIR4_G_S("eth1", "Ethernet1", "ups/dir", "/system/network",
                           eth_getter, eth_setter),
    XPR_UPS_ENTRY_PAR_STR_DV("mac", "", DV_ETH1_MAC),
    // /system/network/eth1/ipv4
    XPR_UPS_ENTRY_DIR4_G_S("ipv4", "IPv4", "ups/dir", NULL, ipv4_getter,
                           ipv4_setter),
    XPR_UPS_ENTRY_PAR_STR_DV("address", "", DV_ETH1_IPV4_ADDRESS),
    XPR_UPS_ENTRY_PAR_STR_DV("netmask", "", DV_ETH1_IPV4_NETMASK),
    XPR_UPS_ENTRY_PAR_STR_DV("gateway", "", DV_ETH1_IPV4_GATEWAY),
    XPR_UPS_ENTRY_PAR_STR_DV("dns1", "", DV_ETH1_IPV4_DNS1),
    XPR_UPS_ENTRY_PAR_STR_DV("dns2", "", DV_ETH1_IPV4_DNS2),
    XPR_UPS_ENTRY_PAR_I32("$perform", "Perform IPv4 Changes"),
    // /system/network/eth1/ipv6
    XPR_UPS_ENTRY_DIR4_G_S("ipv6", "IPv6", "ups/dir", "/system/network/eth1",
                           ipv6_getter, ipv6_setter),
    // /system/network/http
    XPR_UPS_ENTRY_DIR4_G_S("http", "HTTP Protocol", "ups/dir",
                           "/system/network", NULL, NULL),
    XPR_UPS_ENTRY_PAR_I32_DV("enabled", "", DV_HTTP_ENABLED),
    XPR_UPS_ENTRY_PAR_I32_DV("port", "", DV_HTTP_PORT),
    // /system/network/https
    XPR_UPS_ENTRY_DIR4_G_S("https", "HTTPS Protocol", "ups/dir",
                           "/system/network", NULL, NULL),
    XPR_UPS_ENTRY_PAR_I32_DV("enabled", "", DV_HTTPS_ENABLED),
    XPR_UPS_ENTRY_PAR_I32_DV("port", "", DV_HTTPS_PORT),
    // /system/network/ntp
    XPR_UPS_ENTRY_DIR4_G_S("ntp", "NTP Protocol", "ups/dir", "/system/network",
                           NULL, NULL),
    XPR_UPS_ENTRY_PAR_I32_DV("enabled", "", DV_NTP_ENABLED),
    XPR_UPS_ENTRY_PAR_STR_DV("server_address", "", DV_NTP_SERVER_ADDRESS),
    XPR_UPS_ENTRY_PAR_I32_DV("server_port", "", DV_NTP_SERVER_PORT),
    // /system/network/rtsp
    XPR_UPS_ENTRY_DIR4_G_S("rtsp", "RTSP Protocol", "ups/dir",
                           "/system/network", NULL, NULL),
    XPR_UPS_ENTRY_PAR_I32_DV("enabled", "", DV_RTSP_ENABLED),
    XPR_UPS_ENTRY_PAR_I32_DV("port", "", DV_RTSP_PORT),
    XPR_UPS_ENTRY_END(),
};

const int __xpr_ups_driver_sys_net_count =  _countof(__xpr_ups_driver_sys_net);
