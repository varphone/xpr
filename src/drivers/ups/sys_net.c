#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <xpr/xpr_ups.h>
#include <xpr/xpr_utils.h>


static int network_common_get(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key, void* buffer, int* size)
{
	const char *s = NULL;
	int result = 0, len=0;	
    //printf("network_get_ipv4....%s %s\n", key, buffer);

	switch(ent->type) {
		case XPR_UPS_ENTRY_TYPE_BOOLEAN:
			*(int*)buffer = XPR_JSON_BooleanValue(json);
			break;
		case XPR_UPS_ENTRY_TYPE_BLOB:
			// not support yet...
			break;
		case XPR_UPS_ENTRY_TYPE_INT:
			*(int*)buffer = XPR_JSON_IntegerValue(json);	
			break;
		case XPR_UPS_ENTRY_TYPE_INT64:
			*(int64_t*)buffer = XPR_JSON_Integer64Value(json);
			break;
		case XPR_UPS_ENTRY_TYPE_REAL:
			*(double*)buffer = XPR_JSON_RealValue(json);
			break;
		case XPR_UPS_ENTRY_TYPE_STRING:
			s = XPR_JSON_StringValue(json);
			len = strlen(s);
			if(len >= *size) {
				result =  -1;
				break;
			}	
			strcpy_s(buffer, *size, s);
			*size = len;
			break;
		default:
			return -1;
	}
	return result;
}

static int network_set_ipv4(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key, const void* data, int size)
{
	int result = -1;
	int ipv4[4];

    //printf("network_set_ipv4....%s %s\n", key, data);
	//if(sscanf(data, "%d.%d.%d.%d", &ipv4[0],&ipv4[1],&ipv4[2],&ipv4[3]) !=4)
	//	return -1;
#if 0
	char cmd[1024] = {0};
	if(strcmp(s, "address") ==0)
		sprintf(cmd, "ifconfig eth0 %s", (char*)data);
	else 
		sprintf(cmd, "ifconfig eth0 %s %s", s, (char*)data);
	
	if(system(cmd)!=0)
		return 0;
#endif


	switch(ent->type) {
		case XPR_UPS_ENTRY_TYPE_BOOLEAN:
//			result = XPR_JSON_BooleanSet(js, *(int*)buffer);
			break;
		case XPR_UPS_ENTRY_TYPE_BLOB:
			// not support yet...
			break;
		case XPR_UPS_ENTRY_TYPE_INT:
			result = XPR_JSON_IntegerSet(json, *(int*)data);	
			break;
		case XPR_UPS_ENTRY_TYPE_INT64:
			result = XPR_JSON_Integer64Set(json, *(int64_t*)data);
			break;
		case XPR_UPS_ENTRY_TYPE_REAL:
			result = XPR_JSON_RealSet(json, *(double*)data);
			break;
		case XPR_UPS_ENTRY_TYPE_STRING:
            printf("js_json=%s data=%s\n",  XPR_JSON_DumpString(json), data);
			result = XPR_JSON_StringSet(json, (char *)data);
			break;
		default:
			return -1;
	}
	
	// we need to save it to file....
	return result ==0 ? 0 : -1;
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

static const char* xpr_ups_driver_sys_net_ntp_strings_names[] = {"serveradress", 0};
static const char* xpr_ups_driver_sys_net_ntp_strings_descs[] = {"serveradress", 0};

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
