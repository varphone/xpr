#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <net/route.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <linux/sockios.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_ups.h>
#include <xpr/xpr_json.h>
#include <xpr/xpr_utils.h>
//#include <xpr/xpr_devcaps.h>

#define IPC_ROUTE_FILE "/proc/net/route"
#define IPC_RESOLV_FILE "/tmp/resolv.conf"
#define NTP_CONF  "/local/ntp.conf"

extern int XPR_UPS_FindEntry(const char* key, XPR_JSON** json, XPR_UPS_Entry** entry);

struct network
{
	int  dhcp;
	char address[16];
	char netmask[16];
	char gateway[16];
	char dns[64];
};

static int get_address(const char* dev, char* buffer, int size)
{
    int fd = -1;
    char* address = 0;
    struct ifreq ifr;
    struct sockaddr_in* sa;
    //
    strcpy(ifr.ifr_name, dev);
    //
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        fprintf(stderr, "IPC_NET: socket() failure, errno: %d\n", errno);
        return -1;
    }
    if (ioctl(fd, SIOCGIFADDR, &ifr) == -1) {
        fprintf(stderr, "IPC_NET: ioctl(SIOCGIFADDR) failure, errno: %d\n", errno);
        return -1;
    }
    sa = (struct sockaddr_in*)(&ifr.ifr_addr);
    address = inet_ntoa(sa->sin_addr);
    strcpy_s(buffer, size, address);

    close(fd);
    return 0;
}

static int set_address(const char* dev, const char* addr)
{
    int fd = -1;
    char* address = 0;
    struct ifreq ifr;
    struct sockaddr_in* sa;
    //
    strcpy(ifr.ifr_name, dev);
    sa =(struct sockaddr_in*)(&ifr.ifr_addr);
    sa->sin_family = AF_INET;
    sa->sin_port = 0;
    sa->sin_addr.s_addr = inet_addr(addr);
    //
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        fprintf(stderr, "IPC_NET: socket() failure, errno: %d\n", errno);
        return -1;
    }
    if (ioctl(fd, SIOCSIFADDR, &ifr) == -1) {
        fprintf(stderr, "IPC_NET: ioctl(SIOCSIFADDR) failure, errno: %d\n", errno);
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

static int get_netmask(const char* dev, char* buffer, int size)
{
    int fd = -1;
    char* address = 0;
    struct ifreq ifr;
    struct sockaddr_in* sa;
    //
    strcpy(ifr.ifr_name, dev);
    //
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        fprintf(stderr, "IPC_NET: socket() failure, errno: %d\n", errno);
        return -1;
    }
    if( ioctl(fd, SIOCGIFNETMASK, &ifr) == -1) {
        fprintf(stderr, "IPC_NET: ioctl(SIOCGIFNETMASK) failure, errno: %d\n", errno);
        close(fd);
        return -1;
    }

    sa = (struct sockaddr_in *)&ifr.ifr_addr;
    address = inet_ntoa(sa->sin_addr);
    strcpy_s(buffer,size, address);

    close(fd);
    return 0;
}

static int set_netmask(const char* dev, const char* netmask)
{
    int fd = -1;
    char* address = 0;
    struct ifreq ifr;
    struct sockaddr_in* sa;
    //
    strcpy(ifr.ifr_name, dev);
    sa = (struct sockaddr_in*)&ifr.ifr_addr;
    sa->sin_family = AF_INET;
    sa->sin_port = 0;
    sa->sin_addr.s_addr = inet_addr(netmask);
    //
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        fprintf(stderr, "IPC_NET: socket() failure, errno: %d\n", errno);
        return -1;
    }
    if (ioctl(fd, SIOCSIFNETMASK, &ifr) == -1) {
        fprintf(stderr, "IPC_NET: ioctl(SIOCSIFNETMASK) failure, errno: %d\n", errno);
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

static int get_dns(char* buffer, int size)
{
    int fd = 0;
    int l = 0;
    char dns[256];
    char tmp[4096];
    char* s = 0;
    char* sp = 0;
    char* str = 0;
    //
    buffer[0] = 0;
    //
    fd = open(IPC_RESOLV_FILE, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "IPC_NET: open(%s) failure, errno: %d\n", IPC_RESOLV_FILE, errno);
        return -1;
    }
    l = read(fd, tmp, sizeof(tmp)-1);
    if (l < 0) {
        fprintf(stderr, "IPC_NET: read(%s) failure, errno: %d\n", IPC_RESOLV_FILE, errno);
        close(fd);
        return -1;
    }
    tmp[l] = 0;
    sp = tmp;
    for (str = tmp; ; str = NULL) {
        s = strtok_r(str, "\r\n", &sp);
        if (s == NULL)
            break;
        if (sscanf(s, "nameserver %s", dns) < 1)
            continue;
        if (buffer[0])
            strcat(buffer, ",");
        strcat(buffer, dns);
    }
    close(fd);
    return 0;
}

static int set_dns(const char* dns, const char* delim)
{
    int fd = 0;
    char* s = 0;
    char* sp = 0;
    char* str = 0;
    char* tmp = strdup(dns);
    //
    delim = delim ? delim : ",; ";
    fd = open(IPC_RESOLV_FILE, O_WRONLY|O_CREAT|O_TRUNC);
    if (fd < 0) {
        fprintf(stderr, "IPC_NET: open(%s) failure, errno: %d\n", IPC_RESOLV_FILE, errno);
        return -1;
    }
    //
    for (str = tmp; ; str = NULL) {
        s = strtok_r(str, delim, &sp);
        if (s == NULL)
            break;
        write(fd, "nameserver ", 11);
        write(fd, s, strlen(s));
        write(fd, "\n", 1);
    }
    close(fd);
    free(tmp);
    return 0;
}

static int get_gateway(const char* dev, char* buffer, int size)
{
    int fd = 0;
    int l = 0;
    uint32_t dstAddr;
    uint32_t gwAddr;
    char tmp[4096];
    const char* p = 0;
    const char* sp = 0;
    //
    buffer[0] = 0;
    //
    fd = open(IPC_ROUTE_FILE, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "IPC_NET: open(%s) failure, errno: %d\n", IPC_ROUTE_FILE, errno);
        return -1;
    }
    l = read(fd, tmp, sizeof(tmp)-1);
    if (l < 0) {
        fprintf(stderr, "IPC_NET: read(%s) failure, errno: %d\n", IPC_ROUTE_FILE, errno);
        close(fd);
        return -1;
    }
    tmp[l] = 0;
    sp = tmp;
    p = strstr(sp, dev);
    while (p) {
        if (sscanf(p, "%*s %08X %08X", &dstAddr, &gwAddr) < 2)
            break;
        if (dstAddr == 0x00000000) {
            snprintf(buffer, size, "%d.%d.%d.%d",
                     (gwAddr >>  0) & 0xff,
                     (gwAddr >>  8) & 0xff,
                     (gwAddr >> 16) & 0xff,
                     (gwAddr >> 24) & 0xff);
            break;
        }
        sp += 4;
        p = strstr(sp, dev);
    }
    close(fd);
    return 0;
}

static int set_gateway(const char* dev, const char* gateway)
{
    int fd = -1;
    int err = 0;
    struct rtentry rt;    
    //
    fd = socket(AF_INET, SOCK_DGRAM, 0);    
    if (fd < 0) {
        fprintf(stderr, "IPC_NET: socket() failure, errno: %d\n", errno);
        return -1;
    }
    // Delete existing defalt gateway
    memset(&rt, 0, sizeof(rt));
    rt.rt_dst.sa_family = AF_INET;
    ((struct sockaddr_in*)&rt.rt_dst)->sin_addr.s_addr = 0;
    rt.rt_genmask.sa_family = AF_INET;
    ((struct sockaddr_in*)&rt.rt_genmask)->sin_addr.s_addr = 0;
    rt.rt_flags = RTF_UP;
    err = ioctl(fd, SIOCDELRT, &rt);
    if (err < 0) {
        fprintf(stderr, "IPC_NET: ioctl(SIOCDELRT) failure, errno: %d\n", errno);
    }
    if (err == 0 || errno == ESRCH) {
        // Set default gateway
        memset(&rt, 0, sizeof(rt));    
        rt.rt_dst.sa_family = AF_INET;
        ((struct sockaddr_in *)&rt.rt_dst)->sin_addr.s_addr = 0;    
        rt.rt_gateway.sa_family = AF_INET;    
        ((struct sockaddr_in *)&rt.rt_gateway)->sin_addr.s_addr = inet_addr(gateway);    
        rt.rt_genmask.sa_family = AF_INET;    
        ((struct sockaddr_in*)&rt.rt_genmask)->sin_addr.s_addr = 0;    
        rt.rt_flags = RTF_UP | RTF_GATEWAY;
        if ((err = ioctl(fd, SIOCADDRT, &rt)) < 0) {
            fprintf(stderr, "IPC_NET: ioctl(SIOCDELRT) failure, errno: %d\n", errno);
        }
    }
    //
    close(fd);
    return err;
}

#define NETDEV_CTL_DOWN     0
#define NETDEV_CTL_UP       1
static int netdev_ctl(const char* dev, int code)
{
    int fd;
    struct ifreq ifr;
    if (dev == NULL) {
        return -1;
    }
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        fprintf(stderr, "IPC_NET: socket() failure, errno: %d\n", errno);
        return -1;
    }
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, (const char*)dev, IFNAMSIZ - 1);
    if (ioctl(fd, SIOCGIFFLAGS, &ifr) != 0) {
        fprintf(stderr, "IPC_NET: ioctl(SIOCGIFFLAGS) failure, errno: %d\n", errno);
        close(fd);
        return -1;
    }
    switch (code) {
    case NETDEV_CTL_DOWN:
        ifr.ifr_flags &= ~IFF_UP;
        break;
    case NETDEV_CTL_UP:
        ifr.ifr_flags |=  IFF_UP;
        break;
    default:
        break;
    }
    if (ioctl(fd, SIOCSIFFLAGS, &ifr) != 0) {
        fprintf(stderr, "IPC_NET: ioctl(SIOCSIFFLAGS) failure, errno: %d\n", errno);
        close(fd);
        return -1;
    }
    close(fd);
    return 0;
}

static int validate_hwaddr(const char* addr, const char divider)
{
    char ch = 0;
    char ff[3] = {0};
    int i = 0;
    int len = (divider == 0) ? 12 : 17;
    if (strlen(addr) != len)
        return -1;
    for (i = 0; i < len; i++) {
        ch = addr[i];
        if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F') || (ch == divider))
            continue;
        return -1;
    }
    ff[0] = addr[0];
    ff[1] = addr[1];
    if (strtoul(ff, NULL, 16) & 0x01) 
        return -1;
    return 0;
}

int get_hwaddr(const char* dev, char* buffer, int size)
{
    int fd = -1;
    struct ifreq ifr;
    if (dev == NULL || buffer == NULL || size < 18) {
        return -1;
    }
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd <= 0) {
        fprintf(stderr, "IPC_NET: socket() failure, errno: %d\n", errno);
        return -1;
    }
    strncpy(ifr.ifr_name, dev, sizeof(ifr.ifr_name));
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) != 0) {
        fprintf(stderr, "IPC_NET: ioctl(SIOCGIFHWADDR) failure, errno: %d\n", errno);
        close(fd);
        return -1;
    }
    close(fd);
    snprintf(buffer, size,
             "%02X:%02X:%02X:%02X:%02X:%02X",
             (unsigned char)ifr.ifr_hwaddr.sa_data[0],
             (unsigned char)ifr.ifr_hwaddr.sa_data[1],
             (unsigned char)ifr.ifr_hwaddr.sa_data[2],
             (unsigned char)ifr.ifr_hwaddr.sa_data[3],
             (unsigned char)ifr.ifr_hwaddr.sa_data[4],
             (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
    return 0;
}

static int set_hwaddr(const char* dev, const char* addr)
{
    int fd = -1;
    unsigned char bs[8];
    struct ifreq ifr;
    if (dev == NULL || addr == NULL) {
        return -1;
    }

    if (validate_hwaddr(addr, ':') != 0) {
        fprintf(stderr, "IPC_NET: hwaddr [%s] invalid\n", addr);
        return -1;
    }

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd <= 0) {
        fprintf(stderr, "IPC_NET: socket() failure, errno: %d\n", errno);
        return -1;
    }
    ifr.ifr_addr.sa_family = ARPHRD_ETHER;
    strncpy(ifr.ifr_name, (const char*)dev, IFNAMSIZ - 1);
    if (sscanf(addr, "%02hhX:%02hhX:%02hhX:%02hhX:%02hhX:%02hhX",
               &bs[0], &bs[1], &bs[2], &bs[3], &bs[4], &bs[5]) != 6) {
        close(fd);
        return -1;
    }
    memcpy((unsigned char*)ifr.ifr_hwaddr.sa_data, bs, 6);
    //
    netdev_ctl(dev, NETDEV_CTL_DOWN);
    if (ioctl(fd, SIOCSIFHWADDR, &ifr) != 0) {
        fprintf(stderr, "IPC_NET: ioctl(SIOCSIFHWADDR) failure, errno: %d\n", errno);
        close(fd);
        return (-1);
    }
    netdev_ctl(dev, NETDEV_CTL_UP);
    close(fd);
    return 0;
}

static int set_network(const char* dev, const struct network *network)
{
	int result;

	if(!network)
		return -1;
	
	if(network->dhcp) 
		return XPR_System("udhcpc -i eth0 -t 5 -T 1 -f -n -q >& /dev/null");
	
	//ipaddress
	result = set_address(dev, network->address);
    fprintf(stderr, "IPC_NET: set address: %s %s\n", network->address,
        result ? "failure" : "success");

	// netmask
	result = set_netmask(dev, network->netmask);
    fprintf(stderr, "IPC_NET: set netmask: %s %s\n", network->netmask,
        result ? "failure" : "success");

	 // gateway
    result = set_gateway(dev, network->gateway);
    fprintf(stderr, "IPC_NET: set gateway: %s %s\n", network->gateway,
            result ? "failure" : "success");
    // dns
    result = set_dns(network->dns, NULL);
    fprintf(stderr, "IPC_NET: set dns: %s %s\n",network->dns,
            result ? "failure" : "success");

	return 0;
}

static int get_network_from_json(XPR_JSON* json, struct network *network)
{
	XPR_JSON* child_json = NULL;

	if(!json || !network)
		return -1;

	child_json =  XPR_JSON_ObjectGet(json, "dhcp");
	network->dhcp = XPR_JSON_IntegerValue(child_json);
    child_json =  XPR_JSON_ObjectGet(json, "address");
	strcpy_s(network->address,sizeof(network->address), XPR_JSON_StringValue(child_json));
	child_json =  XPR_JSON_ObjectGet(json, "netmask");
	strcpy_s(network->netmask,sizeof(network->netmask), XPR_JSON_StringValue(child_json));
	child_json =  XPR_JSON_ObjectGet(json, "gateway");
	strcpy_s(network->gateway, sizeof(network->gateway),XPR_JSON_StringValue(child_json));
	child_json =  XPR_JSON_ObjectGet(json, "dns1");
	strcpy_s(network->dns, sizeof(network->dns), XPR_JSON_StringValue(child_json));
	return 0;
}


static int network_eth0_ipv4_begin_group(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key, void* buffer, int* size)
{
    if(!ent || !json)
		return XPR_ERR_ERROR;
    ent->data = XPR_JSON_DeepCopy(json);
    return  ent->data ? XPR_ERR_SUCCESS : XPR_ERR_ERROR;
}

static int network_eth0_ipv4_end_group(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key, const void* data, int size)
{
    int dhcp, result = 0;
    char ether[64] = {0};
    XPR_JSON* child_json = NULL;
	struct network network;

    if (ent->data) {
        do {
			get_network_from_json((XPR_JSON*)ent->data, &network);
			child_json =  XPR_JSON_ObjectGet(json, "dhcp");
			dhcp = XPR_JSON_IntegerValue(child_json);
			if(dhcp== 1 && dhcp == network.dhcp) {
				result = 0;
				break;
			}
            if(get_ether_name_from_key(key, ether, sizeof(ether)) !=0)
                return XPR_ERR_ERROR;
            printf("ether is %s \n", ether);
            result = set_network(ether, &network);
			if(result !=0)
				break;
            child_json =  XPR_JSON_ObjectGet(json, "dhcp");
			XPR_JSON_IntegerSet(child_json, network.dhcp);
            child_json =  XPR_JSON_ObjectGet(json, "address");
			XPR_JSON_StringSet(child_json, network.address);
            child_json =  XPR_JSON_ObjectGet(json, "netmask");
            XPR_JSON_StringSet(child_json, network.netmask);
            child_json =  XPR_JSON_ObjectGet(json, "gateway");
            XPR_JSON_StringSet(child_json,network.gateway);
            child_json =  XPR_JSON_ObjectGet(json, "dns1");
            XPR_JSON_StringSet(child_json, network.dns);
            result = 0;
			break;
        } while(0);
        XPR_JSON_DecRef(ent->data);//和  XPR_JSON_DeepCopy()对应
        ent->data=NULL;
		if(result==0)
	        XPR_UPS_Sync();//同步更新文件
    }
    return result ==0 ? XPR_ERR_SUCCESS : XPR_ERR_ERROR;
}


static char* ipc_net_post_scripts = ""
"route add -net 127.0.0.0 netmask 255.0.0.0 dev lo >& /dev/null;"
"route add -net 224.0.0.0 netmask 240.0.0.0 dev eth0 >& /dev/null;";

static int start_ntp(int status, const char* address)
{
#if 0
	int result, enable;
    XPR_JSON* ntp_json = NULL, *child_json =NULL;
    XPR_UPS_Entry* ntp_entry = NULL;
	char server_address[128],cmd[1024];

	result = XPR_UPS_FindEntry("/system/network/ntp/", &ntp_json, &ntp_entry);
	if(result !=XPR_ERR_SUCCESS) {
		fprintf(stderr, "get ntp/ failed\n");
		return -1;
	}

	child_json =  XPR_JSON_ObjectGet(ntp_json, "enabled");
	enable = XPR_JSON_IntegerValue(child_json);
	if(!enable || (enable && status==1))
		return 0;

	child_json =  XPR_JSON_ObjectGet(ntp_json, "server_address");
	strcpy_s(server_address, sizeof(server_address), XPR_JSON_StringValue(child_json));
	if(!address) 
		snprintf(cmd, sizeof(cmd), "ntpd -N -p %s >& /dev/null &", server_address);
	else if(strcmp(server_address, address) != 0)
		snprintf(cmd, sizeof(cmd), "ntpd -N -p %s >& /dev/null &", address);
	else
		return 0;

	XPR_System("killall ntpd >& /dev/null");
	XPR_System(cmd);	
	return 0;
#else
	char buffer[1024] = {0};
	if(address !=0) {
		int fd = open(NTP_CONF, O_WRONLY|O_CREAT|O_TRUNC);
		if(fd <0) {
			printf("can not open");
			return -1;
		}

		snprintf(buffer, sizeof(buffer), "server %s", address);
		write(fd, buffer, sizeof(buffer));
		close(fd);
	}
	snprintf(buffer, sizeof(buffer), "/sbin/ntpd -c %s >& /dev/null", NTP_CONF);
	XPR_System("killall ntpd >& /dev/null");
	XPR_System(buffer);
	return 0;

#endif
}

static int stop_ntp()
{
	XPR_System("killall ntpd >& /dev/null");
	return 0;
}

static int init_plugin_network(XPR_UPS_Entry* ent)
{
	char mac[32] = {0};
	int result, dhcp=0, dhcp_ok = 0;
    int size;
	struct network network;
    XPR_JSON* json = NULL;
    XPR_UPS_Entry* entry = NULL;

	
#if 1
	set_address("lo", "127.0.0.1");
    set_netmask("lo", "255.0.0.0");

	size = sizeof(mac);
    XPR_UPS_GetString("/system/network/eth0/mac", mac, &size);
    result = set_hwaddr("eth0", mac);
	fprintf(stderr, "IPC_NET: set hwaddress: %s %s\n", mac,
	        result ? "failure" : "success");

	result = XPR_UPS_GetInteger("/system/network/eth0/ipv4/dhcp", &dhcp);
	if(dhcp) {
		network.dhcp = dhcp;
        if(set_network("eth0", &network) ==0)
			dhcp_ok = 1;
		else 
			dhcp_ok = 0;
	}
	if(dhcp == 0 || dhcp_ok==0) {
		result = XPR_UPS_FindEntry("/system/network/eth0/ipv4/", &json, &entry);
		if(result !=XPR_ERR_SUCCESS) { 
			strcpy_s(network.address, sizeof(network.address), "192.168.3.99");
			strcpy_s(network.netmask, sizeof(network.netmask), "255.255.0.0");
			strcpy_s(network.gateway, sizeof(network.gateway), "192.168.0.1");
		}
		else {
			get_network_from_json(json, &network);		
		}
		network.dhcp = 0;
        set_network("eth0", &network);
	}
	XPR_System(ipc_net_post_scripts);
	start_ntp(0, NULL);
#endif
    return XPR_ERR_SUCCESS;
}

static int fini_plugin_network(XPR_UPS_Entry* ent)
{
    return XPR_ERR_SUCCESS;
}

static int network_common_get(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key, void* buffer, int* size)
{
  return XPR_UPS_ReadData(ent, json, key, buffer, size);
}

static int network_set_ntp(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key, const void* data, int size)
{
	int result, enable;
	char name[128];

	if(get_action_from_key(key, name, sizeof(name)) !=0) 
        return XPR_ERR_ERROR;

	if(strcmp(name, "enabled") == 0) {
		enable = *((int*)data);
		result = enable ? start_ntp(enable, NULL) : stop_ntp();
	}
	else if(strcmp(name, "server_port")==0) {
		result = 0;
	}
	else if(strcmp(name, "server_address")==0) {
		result = start_ntp(0, (char*)data);
	}
	else {
		result = -1;
	}
	
	return result ==0 ? XPR_UPS_WriteData(ent, json, key, data, size) : XPR_ERR_ERROR;
}

static int network_get_ipv4(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key, void* buffer, int* size)
{
	char name[128],mac_address[32], ether[64], fmt[1024];
	uint8_t serial[8];
	int result, dhcp = 0;
    XPR_JSON* dhcp_json = NULL;
    XPR_UPS_Entry* dhcp_entry = NULL;
	struct timeval start, end;
	double interval;


    if(get_action_from_key(key, name, sizeof(name)) !=0) 
        return XPR_ERR_ERROR;

	if(get_ether_name_from_key(key, ether, sizeof(ether)) !=0)
	        return XPR_ERR_ERROR;
	
	/*if(strcmp(name, "mac") == 0) {
		result = XPR_DevCapsGetString(XPR_DEVCAPS_ID_SERIAL, 0, (char*)serial, sizeof(serial));
		if(result != XPR_ERR_SUCCESS)
			return result;
		snprintf(mac_address, sizeof(mac_address), "12:15:%02X:%02X:%02X:%02X", serial[3], serial[2], serial[1], serial[0]);
		strcpy_s((char*)buffer, *size, mac_address);
		return XPR_ERR_SUCCESS;
	}*/
	snprintf(fmt, sizeof(fmt), "/system/network/%s/ipv4/dhcp", ether);
	result = XPR_UPS_FindEntry(fmt, &dhcp_json, &dhcp_entry);
	dhcp = result == XPR_ERR_SUCCESS ? XPR_JSON_IntegerValue(dhcp_json) : 0 ;

	if(dhcp) {
		if(strcmp(name, "address") ==0) 
             result = get_address(ether, buffer, *size);
        else if(strcmp(name, "netmask") ==0) 
            result = get_netmask(ether, buffer, *size);
		else if(strcmp(name, "gateway") ==0) 
            result = get_gateway(ether, buffer, *size);
		else if(strcmp(name, "dns1") ==0)
			result = get_dns(buffer, *size);
		else if(strcmp(name, "dhcp")==0)
			return XPR_UPS_ReadData(ent, json, key, buffer, size); 
		return result == 0 ? XPR_ERR_SUCCESS : XPR_ERR_ERROR;
	}

    return XPR_UPS_ReadData(ent, json, key, buffer, size);
}

static int network_set_ipv4(XPR_UPS_Entry* ent, XPR_JSON* json, const char* key, const void* data, int size)
{
	int result = XPR_ERR_ERROR;
    int ipv4[4],dhcp, dhcp_new=0;
    char name[128] = {0}, ether[64], fmt[1024];
	struct network network;
	XPR_JSON* child_json;
    XPR_JSON* dhcp_json = NULL;
    XPR_UPS_Entry* dhcp_entry = NULL;

    if(get_action_from_key(key, name, sizeof(name)) !=0) 
        return XPR_ERR_ERROR;

	if(get_ether_name_from_key(key, ether, sizeof(ether)) !=0)
	    return XPR_ERR_ERROR;

	if(strcmp(ether, "eth0") !=0 && strcmp(ether, "eth1") !=0)
		return XPR_ERR_ERROR;
	
    if(ent->parent && ent->parent->data) {
		child_json =  XPR_JSON_ObjectGet((XPR_JSON*)ent->parent->data, name);
        if(ent->type == XPR_UPS_ENTRY_TYPE_STRING) 
            XPR_JSON_StringSet(child_json, (char*)data);
		else 
            XPR_JSON_IntegerSet(child_json, *(int*)data);
        return XPR_ERR_PEDDING;
    }
	snprintf(fmt, sizeof(fmt), "/system/network/%s/ipv4/", ether);
	result = XPR_UPS_FindEntry(fmt, &dhcp_json, &dhcp_entry);
	if(result !=XPR_ERR_SUCCESS) 
		return result;
	child_json =  XPR_JSON_ObjectGet(dhcp_json, "dhcp");
	dhcp = XPR_JSON_IntegerValue(child_json);

    if(strcmp(name, "mac") ==0) {
            return XPR_ERR_UPS_NOT_SUPPORT;
    }
    else if(strcmp(name, "address")==0) {
        if(sscanf(data, "%d.%d.%d.%d", &ipv4[0], &ipv4[1], &ipv4[2], &ipv4[3]) !=4)
            return XPR_ERR_UPS_ILLEGAL_PARAM;
        result = set_address(ether, (char*)data);
    }
    else if( strcmp(name, "netmask")==0) {
        if(sscanf(data, "%d.%d.%d.%d", &ipv4[0], &ipv4[1], &ipv4[2], &ipv4[3]) !=4)
            return XPR_ERR_UPS_ILLEGAL_PARAM;
        result = set_netmask(ether, (char*)data);
    }
    else if(strcmp(name, "gateway")==0) {
        if(sscanf(data, "%d.%d.%d.%d", &ipv4[0], &ipv4[1], &ipv4[2], &ipv4[3]) !=4)
            return XPR_ERR_UPS_ILLEGAL_PARAM;
        result = set_gateway(ether, (char*)data);
    }
	else if(strcmp(name, "dns1")==0) {
		result = set_dns((char*)data, NULL);
	}
	else if(strcmp(name, "dhcp")==0) {
		dhcp_new = *((int*)data);
		if(dhcp == dhcp_new) 
			return XPR_ERR_SUCCESS;
		if(dhcp_new == 0) {
			get_network_from_json(dhcp_json, &network);
			network.dhcp = 0;
		}
		else {
			network.dhcp = 1;
		}
        result = set_network(ether, &network);
		if(result !=0 && dhcp_new) {
			network.dhcp = 0;
			get_network_from_json(dhcp_json, &network);
            set_network(ether, &network);
		} 
	}
	else {
		result = -1;
	}
	XPR_System(ipc_net_post_scripts);
	if(dhcp_new && result==0)
		fprintf(stderr, "we will write dhcp \n");
	return result == 0 ? XPR_UPS_WriteData(ent, json, key, data, size) : XPR_ERR_ERROR;
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

static const char* xpr_ups_driver_sys_net_ntp_strings_names[] = {"server_address", 0};
static const char* xpr_ups_driver_sys_net_ntp_strings_descs[] = {"server_address", 0};

static const char* xpr_ups_driver_sys_net_ntp_bool_names[] = {"enabled", 0};
static const char* xpr_ups_driver_sys_net_ntp_bool_descs[] = {"enabled", 0};

static const char* xpr_ups_driver_sys_net_ntp_ints_names[] = {"server_port", 0};
static const char* xpr_ups_driver_sys_net_ntp_ints_descs[] = {"server_port", 0};

static const char* xpr_ups_driver_sys_net_ints_namnes[] = {"http_port", "https_port", "rtsp_port", "http_enabled", "https_enabled", "rtsp_enabled", 0};
static const char* xpr_ups_driver_sys_net_ints_descs[] =  {"http_port", "https_port", "rtsp_port", "http_enabled", "https_enabled", "rtsp_enabled", 0};

static const char* xpr_ups_driver_sys_net_eth0_names[] = { "eth0", 0};
static const char* xpr_ups_driver_sys_net_eth0_descs[] = { "network interface card 0", 0};

static const char* xpr_ups_driver_sys_net_eth0_strings_names[] = { "name", "mac", 0};
static const char* xpr_ups_driver_sys_net_eth0_strings_descs[] = { "ethernet 0 name", "mac address", 0};

static const char* xpr_ups_driver_sys_net_eth1_names[] = { "eth1", 0};
static const char* xpr_ups_driver_sys_net_eth1_descs[] = { "network interface card 1", 0};

static const char* xpr_ups_driver_sys_net_eth1_strings_names[] = { "name", "mac", 0};
static const char* xpr_ups_driver_sys_net_eth1_strings_descs[] = { "ethernet 1 name", "mac address", 0};

static const char* xpr_ups_driver_sys_net_eth_ipv4_names[] = { "ipv4", 0};
static const char* xpr_ups_driver_sys_net_eth_ipv4_descs[] = { "ipv4", 0};

static const char* xpr_ups_driver_sys_net_eth_ipv4_bool_names[] = {"dhcp", 0};
static const char* xpr_ups_driver_sys_net_eth_ipv4_bool_descs[] = {"dhcp", 0};

static const char* xpr_ups_driver_sys_net_eth_ipv4_strings_names[] = { "address", "netmask", "gateway", "dns1", "dns2", 0};
static const char* xpr_ups_driver_sys_net_eth_ipv4_strings_descs[] = { "address", "netmask", "gateway", "dns1", "dns2", 0};


//static const char* xpr_ups_driver_sys_net_eth1_ipv4_names[] = { "ipv4", 0};
//static const char* xpr_ups_driver_sys_net_eth1_ipv4_descs[] = { "ipv4", 0};

//static const char* xpr_ups_driver_sys_net_eth1_ipv4_bool_names[] = {"dhcp", 0};
//static const char* xpr_ups_driver_sys_net_eth1_ipv4_bool_descs[] = {"dhcp", 0};

//static const char* xpr_ups_driver_sys_net_eth1_ipv4_strings_names[] = { "address", "netmask", "gateway", "dns1", "dns2", 0};
//static const char* xpr_ups_driver_sys_net_eth1_ipv4_strings_descs[] = { "address", "netmask", "gateway", "dns1", "dns2", 0};


XPR_UPS_Entry xpr_ups_driver_system_network[] = {
    {
        xpr_ups_driver_sys_names,
        xpr_ups_driver_sys_descs,
        "ups/dir",
        "/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0, 0, 0
    },
    {
        xpr_ups_driver_sys_net_names,
        xpr_ups_driver_sys_net_descs,
        "ups/dir",
        "/system/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0, 0, 0
    },
    {
        xpr_ups_driver_sys_net_ints_namnes,
        xpr_ups_driver_sys_net_ints_descs,
        "ups/entry",
        "/system/network/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, network_common_get, network_set_ipv4,
        0, 0, 0, 0, 0
    },
    {  
        xpr_ups_driver_sys_net_ntp_names,
        xpr_ups_driver_sys_net_ntp_descs,
        "ups/dir",
        "/system/network/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0, 0, 0
    },
    {
        xpr_ups_driver_sys_net_ntp_strings_names,
        xpr_ups_driver_sys_net_ntp_strings_descs,
        "ups/entry",
        "/system/network/ntp/",
        XPR_UPS_ENTRY_TYPE_STRING,
        0, 0, network_common_get, network_set_ntp,
        0, 0, 0, 0, 0
    },
    {
        xpr_ups_driver_sys_net_ntp_bool_names,
        xpr_ups_driver_sys_net_ntp_bool_descs,
        "ups/entry",
        "/system/network/ntp/",
        XPR_UPS_ENTRY_TYPE_BOOLEAN,
        0, 0, network_common_get, network_set_ntp,
        0, 0, 0, 0, 0
    },
    {
        xpr_ups_driver_sys_net_ntp_ints_names,
        xpr_ups_driver_sys_net_ntp_ints_descs,
        "ups/entry",
        "/system/network/ntp/",
        XPR_UPS_ENTRY_TYPE_INT,
        0, 0, network_common_get, network_set_ntp,
        0, 0, 0, 0, 0
    },
    {
        xpr_ups_driver_sys_net_eth0_names,
        xpr_ups_driver_sys_net_eth0_descs,
        "ups/dir",
        "/system/network/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0, 0, 0
    },
    {
        xpr_ups_driver_sys_net_eth0_strings_names,
        xpr_ups_driver_sys_net_eth0_strings_descs,
        "ups/entry",
        "/system/network/eth0/",
        XPR_UPS_ENTRY_TYPE_STRING,
        0, 0, network_get_ipv4, network_set_ipv4,
        0, 0, 0, 0, 0
    },
    {
        xpr_ups_driver_sys_net_eth_ipv4_names,
        xpr_ups_driver_sys_net_eth_ipv4_descs,
        "ups/dir",
        "/system/network/eth0/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, network_eth0_ipv4_begin_group, network_eth0_ipv4_end_group,
        0, 0, 0, 0, 0
    },
    {
        xpr_ups_driver_sys_net_eth_ipv4_bool_names,
        xpr_ups_driver_sys_net_eth_ipv4_bool_descs,
        "ups/entry",
        "/system/network/eth0/ipv4/",
        XPR_UPS_ENTRY_TYPE_BOOLEAN,
        0, 0, network_get_ipv4, network_set_ipv4,
        0, 0, 0, 0, 0
    },
    {
        xpr_ups_driver_sys_net_eth_ipv4_strings_names,
        xpr_ups_driver_sys_net_eth_ipv4_strings_descs,
        "ups/entry",
        "/system/network/eth0/ipv4/",
        XPR_UPS_ENTRY_TYPE_STRING,
        /*init_plugin_network*/0, 0/*fini_plugin_network*/, network_get_ipv4, network_set_ipv4,
        0, 0, 0, 0, 0
    },
// for eth1
    {
        xpr_ups_driver_sys_net_eth1_names,
        xpr_ups_driver_sys_net_eth1_descs,
        "ups/dir",
        "/system/network/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, 0, 0,
        0, 0, 0, 0, 0
    },
    {
        xpr_ups_driver_sys_net_eth1_strings_names,
        xpr_ups_driver_sys_net_eth1_strings_descs,
        "ups/entry",
        "/system/network/eth1/",
        XPR_UPS_ENTRY_TYPE_STRING,
        0, 0, network_get_ipv4, network_set_ipv4,
        0, 0, 0, 0, 0
    },
    {
        xpr_ups_driver_sys_net_eth_ipv4_names,
        xpr_ups_driver_sys_net_eth_ipv4_descs,
        "ups/dir",
        "/system/network/eth1/",
        XPR_UPS_ENTRY_TYPE_DIR,
        0, 0, network_eth0_ipv4_begin_group, network_eth0_ipv4_end_group,
        0, 0, 0, 0, 0
    },
    {
        xpr_ups_driver_sys_net_eth_ipv4_bool_names,
        xpr_ups_driver_sys_net_eth_ipv4_bool_descs,
        "ups/entry",
        "/system/network/eth1/ipv4/",
        XPR_UPS_ENTRY_TYPE_BOOLEAN,
        0, 0, network_get_ipv4, network_set_ipv4,
        0, 0, 0, 0, 0
    },
    {
        xpr_ups_driver_sys_net_eth_ipv4_strings_names,
        xpr_ups_driver_sys_net_eth_ipv4_strings_descs,
        "ups/entry",
        "/system/network/eth1/ipv4/",
        XPR_UPS_ENTRY_TYPE_STRING,
        init_plugin_network, fini_plugin_network, network_get_ipv4, network_set_ipv4,
        0, 0, 0, 0, 0
    },

};

const int xpr_ups_driver_system_network_count =  _countof(xpr_ups_driver_system_network);
