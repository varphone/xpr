#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_ups.h>
#include <xpr/xpr_utils.h>


static int network_common_get(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key, void* buffer, int* size)
{
    return XPR_UPS_ReadData(ent, json, key, buffer, size);
//	const char *s = NULL;
//    int result = 0, len=0;

//	switch(ent->type) {
//		case XPR_UPS_ENTRY_TYPE_BOOLEAN:
//			*(int*)buffer = XPR_JSON_BooleanValue(json);
//			break;
//		case XPR_UPS_ENTRY_TYPE_BLOB:
//			// not support yet...
//			break;
//		case XPR_UPS_ENTRY_TYPE_INT:
//			*(int*)buffer = XPR_JSON_IntegerValue(json);
//			break;
//		case XPR_UPS_ENTRY_TYPE_INT64:
//			*(int64_t*)buffer = XPR_JSON_Integer64Value(json);
//			break;
//		case XPR_UPS_ENTRY_TYPE_REAL:
//			*(double*)buffer = XPR_JSON_RealValue(json);
//			break;
//		case XPR_UPS_ENTRY_TYPE_STRING:
//			s = XPR_JSON_StringValue(json);
//			len = strlen(s);
//			if(len >= *size) {
//                result =  XPR_ERR_BUF_FULL;
//				break;
//			}
//			strcpy_s(buffer, *size, s);
//			*size = len;
//			break;
//		default:
//            return XPR_ERR_ILLEGAL_PARAM;
//	}
//	return result;
}

static int network_set_ipv4(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key, const void* data, int size)
{
	int result = -1;
    int ipv4[4];

    int len = strlen(key);
    if(key[len - 1] == '/')
        return XPR_ERR_UPS_ILLEGAL_PARAM;
    int i = len - 1;
    while(key[i] != '/') {
        --i;
    }
    char node[64] = {0};
    int nodelen = len - 1 - i + 1;
    if(nodelen > 64)
        return XPR_ERR_BUF_FULL;
    strcpy_s(node, nodelen, key+i+1);

    char cmd[128] = {0};

    printf("int data   *data = %d\n", *(int*)data);

    if(ent->type == XPR_UPS_ENTRY_TYPE_STRING) {
        if(0 == strcmp(node, "mac")) {
            snprintf(cmd, sizeof(cmd), "ifconfig eth1 hw ether %s", (char*)data);
            result = system("ifconfig eth1 down");
            result = system(cmd);
            result = system("ifconfig eth1 up");
        }
        else if(0 == strcmp(node, "address")) {
            if(sscanf(data, "%d.%d.%d.%d", &ipv4[0], &ipv4[1], &ipv4[2], &ipv4[3]) !=4)
                return XPR_ERR_UPS_ILLEGAL_PARAM;
            snprintf(cmd, sizeof(cmd), "ifconfig eth1 %s", (char*)data);
            result = system(cmd);
        }
        else if(0 == strcmp(node, "netmask")) {
            if(sscanf(data, "%d.%d.%d.%d", &ipv4[0], &ipv4[1], &ipv4[2], &ipv4[3]) !=4)
                return XPR_ERR_UPS_ILLEGAL_PARAM;
            snprintf(cmd, sizeof(cmd), "ifconfig eth1 netmask %s", (char*)data);
            result = system(cmd);
        }
        else if(0 == strcmp(node, "gateway")) {
            if(sscanf(data, "%d.%d.%d.%d", &ipv4[0], &ipv4[1], &ipv4[2], &ipv4[3]) !=4)
                return XPR_ERR_UPS_ILLEGAL_PARAM;
            snprintf(cmd, sizeof(cmd), "route add default gw %s", (char*)data);
            result = system(cmd);
        }

        if(-1 == result)
            return XPR_ERR_UPS_NOT_SUPPORT;
    }

//    if(ent->type == XPR_UPS_ENTRY_TYPE_BOOLEAN)
//        printf("XPR_UPS_ENTRY_TYPE_BOOLEAN   *data = %d\n", *(int*)data);

    if(-1 == XPR_UPS_WriteData(ent, json, key, data, size))
        return XPR_ERR_UPS_WRITE;

    if(-1 == XPR_UPS_DumpFile())
        return XPR_ERR_UPS_DUMP;
	
    return XPR_ERR_OK;
    // we need to save it to file....
}
/*
	/system/network/eth0/ipv4/address
*/
static const char* xpr_ups_driver_sys_names[] = { "system", 0};
static const char* xpr_ups_driver_sys_descs[] = { "system", 0};

static const char* xpr_ups_driver_sys_net_names[] = { "network", 0};
static const char* xpr_ups_driver_sys_net_descs[] = { "network", 0};

static const char* xpr_ups_driver_sys_net_ntp_names[] = {"ntp", 0};
static const char* xpr_ups_driver_sys_net_ntp_descs[] = {"ntp", 0};

static const char* xpr_ups_driver_sys_net_ntp_strings_names[] = {"serveraddress", 0};
static const char* xpr_ups_driver_sys_net_ntp_strings_descs[] = {"serveraddress", 0};

static const char* xpr_ups_driver_sys_net_ntp_bool_names[] = {"enable", 0};
static const char* xpr_ups_driver_sys_net_ntp_bool_descs[] = {"enable", 0};

static const char* xpr_ups_driver_sys_net_ints_namnes[] = {"httpport","rtspport", 0};
static const char* xpr_ups_driver_sys_net_ints_descs[] =  {"http port","rtsp port", 0};

static const char* xpr_ups_driver_sys_net_eth0_names[] = { "eth0", 0};
static const char* xpr_ups_driver_sys_net_eth0_descs[] = { "network interface card 0", 0};

static const char* xpr_ups_driver_sys_net_eth0_strings_names[] = { "name", "mac", 0};
static const char* xpr_ups_driver_sys_net_eth0_strings_descs[] = { "ethernet name", "mac address", 0};

static const char* xpr_ups_driver_sys_net_eth_ipv4_names[] = { "ipv4", 0};
static const char* xpr_ups_driver_sys_net_eth_ipv4_descs[] = { "ipv4", 0};

static const char* xpr_ups_driver_sys_net_eth_ipv4_bool_names[] = {"dhcp", 0};
static const char* xpr_ups_driver_sys_net_eth_ipv4_bool_descs[] = {"dhcp", 0};

static const char* xpr_ups_driver_sys_net_eth_ipv4_strings_names[] = { "address", "netmask", "gateway", "dns1", "dns2", 0};
static const char* xpr_ups_driver_sys_net_eth_ipv4_strings_descs[] = { "address", "netmask", "gateway", "dns1", "dns2", 0};


XPR_UPS_Entry xpr_ups_driver_system_network[] = {
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
        xpr_ups_driver_sys_net_names,
        xpr_ups_driver_sys_net_descs,
        "ups/dir",
        "/system/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {	
        xpr_ups_driver_sys_net_ints_namnes,
        xpr_ups_driver_sys_net_ints_descs,
        "ups/entry",
        "/system/network/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, network_common_get, network_set_ipv4,
        0, 0, 0
    },
    {	
        xpr_ups_driver_sys_net_ntp_names,
        xpr_ups_driver_sys_net_ntp_descs,
        "ups/dir",
        "/system/network/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_sys_net_ntp_strings_names,
        xpr_ups_driver_sys_net_ntp_strings_descs,
        "ups/entry",
        "/system/network/ntp/",
        XPR_UPS_ENTRY_TYPE_STRING,
        0, 0, network_common_get, network_set_ipv4,
        0, 0, 0
    },
    {
        xpr_ups_driver_sys_net_ntp_bool_names,
        xpr_ups_driver_sys_net_ntp_bool_descs,
        "ups/entry",
        "/system/network/ntp/",
        XPR_UPS_ENTRY_TYPE_BOOLEAN,
        0, 0, network_common_get, network_set_ipv4,
        0, 0, 0
    },
    {
        xpr_ups_driver_sys_net_eth0_names,
        xpr_ups_driver_sys_net_eth0_descs,
        "ups/dir",
        "/system/network/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_sys_net_eth0_strings_names,
        xpr_ups_driver_sys_net_eth0_strings_descs,
        "ups/entry",
        "/system/network/eth0/",
        XPR_UPS_ENTRY_TYPE_STRING,
        0, 0, network_common_get, network_set_ipv4,
        0, 0, 0
    },
    {
        xpr_ups_driver_sys_net_eth_ipv4_names,
        xpr_ups_driver_sys_net_eth_ipv4_descs,
        "ups/dir",
        "/system/network/eth0/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0
    },
    {
        xpr_ups_driver_sys_net_eth_ipv4_bool_names,
        xpr_ups_driver_sys_net_eth_ipv4_bool_descs,
        "ups/entry",
        "/system/network/eth0/ipv4/",
        XPR_UPS_ENTRY_TYPE_BOOLEAN,
        0, 0, network_common_get, network_set_ipv4,
        0, 0, 0
    },
    {
        xpr_ups_driver_sys_net_eth_ipv4_strings_names,
        xpr_ups_driver_sys_net_eth_ipv4_strings_descs,
        "ups/entry",
        "/system/network/eth0/ipv4/",
        XPR_UPS_ENTRY_TYPE_STRING,
        0, 0, network_common_get, network_set_ipv4,
        0, 0, 0
    },

};

const int xpr_ups_driver_system_network_count =  _countof(xpr_ups_driver_system_network);
