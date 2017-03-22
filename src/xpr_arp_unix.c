#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/if_ether.h>
#include <netpacket/packet.h>
#include <arpa/inet.h>
#include <xpr/xpr_common.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_arp.h>

#define ETH_ARP_LEN 42

struct XPR_ARP {
    int sock;
    int if_index;
    unsigned char if_hwaddr[6];
    struct in_addr if_addr;
    unsigned char rx_buf[ETH_FRAME_LEN];
    unsigned char tx_buf[ETH_FRAME_LEN];
};

static int XPR_ARP_Init(XPR_ARP* arp, const char* dev)
{
    struct sockaddr_ll sa;
    struct ifreq ifr;
    struct timeval tmo = { 0, 300000 };
    //
    arp->sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (arp->sock < 0) {
        fprintf(stderr, "XPR_ARP: socket() failure, errno: %d\n", errno);
        return -1;
    }
    //
    if (setsockopt(arp->sock, SOL_SOCKET, SO_SNDTIMEO, &tmo, sizeof(tmo)) < 0) {
        fprintf(stderr, "XPR_ARP: setsockopt() failure, errno: %d\n", errno);
        return -1;
    }
    if (setsockopt(arp->sock, SOL_SOCKET, SO_RCVTIMEO, &tmo, sizeof(tmo)) < 0) {
        fprintf(stderr, "XPR_ARP: setsockopt() failure, errno: %d\n", errno);
        return -1;
    }
    //
    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, dev);
    //
    if (ioctl(arp->sock, SIOCGIFINDEX, &ifr) < 0) {
        return -1;
    }
    arp->if_index = ifr.ifr_ifindex;
    // 获取接口 IP 地址
    if (ioctl(arp->sock, SIOCGIFADDR, &ifr) < 0) {
        return -1;
    }
    arp->if_addr = ((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr;
    //
    if (ioctl(arp->sock, SIOCGIFHWADDR, &ifr) < 0) {
        return -1;
    }
    memcpy(arp->if_hwaddr, ifr.ifr_hwaddr.sa_data, ETH_ALEN);
    //
    sa.sll_family = PF_PACKET;
    sa.sll_protocol = htons(ETH_P_ARP);
    sa.sll_hatype = ARPHRD_ETHER;
    sa.sll_pkttype = PACKET_HOST;
    sa.sll_halen = ETH_ALEN;
    sa.sll_ifindex = arp->if_index;
    memcpy(sa.sll_addr, arp->if_hwaddr, ETH_ALEN);

    if (bind(arp->sock, (struct sockaddr*)&sa, sizeof(sa)) < 0)
        return -1;

    return 0;
}

static int XPR_ARP_Fini(XPR_ARP* arp)
{
    if (arp) {
        if (arp->sock) {
            close(arp->sock);
            arp->sock = -1;
        }
    }
    return XPR_ERR_OK;
}

XPR_API XPR_ARP* XPR_ARP_New(const char* dev)
{
    XPR_ARP* arp = (XPR_ARP*)calloc(sizeof(*arp), 1);
    if (arp) {
        XPR_ARP_Init(arp, dev);
    }
    return arp;
}

XPR_API int XPR_ARP_Destroy(XPR_ARP* arp)
{
    if (arp) {
        if (arp->sock > 0)
            close(arp->sock);
        free(arp);
    }
    return 0;
}

XPR_API int XPR_ARP_Ask(XPR_ARP* arp, const char* host)
{
    static unsigned char bcast_mac[6] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    int n = 0;
    struct ether_header* eth = 0;
    struct ether_arp* eth_arp = 0;
    struct in_addr target_addr = {0};
    struct sockaddr_ll sa = {0};
    // 开始填充，构造以太头部
    eth = (struct ether_header*)arp->tx_buf;
    memcpy(eth->ether_dhost, bcast_mac, ETH_ALEN);
    memcpy(eth->ether_shost, arp->if_hwaddr, ETH_ALEN);
    eth->ether_type = htons(ETHERTYPE_ARP);
    // 手动开始填充用ARP报文首部
    eth_arp = (struct ether_arp*)(arp->tx_buf + sizeof(struct ether_header));
    eth_arp->arp_hrd = htons(ARPHRD_ETHER); //硬件类型为以太
    eth_arp->arp_pro = htons(ETHERTYPE_IP); //协议类型为IP
    //硬件地址长度和IPV4地址长度分别是6字节和4字节
    eth_arp->arp_hln = ETH_ALEN;
    eth_arp->arp_pln = 4;
    //操作码，这里我们发送ARP请求
    eth_arp->arp_op = htons(ARPOP_REQUEST);
    //填充发送端的MAC和IP地址
    memcpy(eth_arp->arp_sha, arp->if_hwaddr, ETH_ALEN);
    memcpy(eth_arp->arp_spa, &arp->if_addr, 4);
    //填充目的端的IP地址，MAC地址不用管
    inet_pton(AF_INET, host, &target_addr);
    memcpy(eth_arp->arp_tpa, &target_addr, 4);
    sa.sll_family = PF_PACKET;
    sa.sll_ifindex = arp->if_index;
    n = sendto(arp->sock, arp->tx_buf, ETH_ARP_LEN, 0,
               (struct sockaddr*)&sa, sizeof(sa));
    return 0;
}

XPR_API int XPR_ARP_Poll(XPR_ARP* arp)
{
    int n = 0;
    n = recvfrom(arp->sock, arp->rx_buf, sizeof(arp->rx_buf), 0, NULL, NULL);
    return n > 0 ? XPR_ERR_OK : XPR_ERR_ERROR;
}

static int XPR_ARP_IsHostExists(XPR_ARP* arp, const char* host)
{
    char tmp[128];
    struct ether_header* eth = (struct ether_header*)arp->rx_buf;
    struct ether_arp* eth_arp = (struct ether_arp*)(arp->rx_buf + sizeof(struct ether_header));
    struct in_addr host_addr = { 0 };
    inet_pton(AF_INET, host, &host_addr);
    if (memcmp(&host_addr, eth_arp->arp_spa, 4) == 0)
        return XPR_TRUE;
    return XPR_FALSE;
}

XPR_API int XPR_ARP_Scan(XPR_ARP* arp, const char* host)
{
    if (XPR_ARP_Ask(arp, host) < 0)
        return XPR_ERR_ERROR;
    while (1) {
        if (XPR_ARP_Poll(arp) < 0)
            break;
        if (XPR_ARP_IsHostExists(arp, host))
            return XPR_ERR_OK;
    }
    return XPR_ERR_ERROR;
}
