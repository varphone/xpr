#include <arpa/inet.h>
#include <errno.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netpacket/packet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <xpr/xpr_common.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_icmp.h>

struct XPR_ICMP
{
    int sock;
    int seq;
    struct sockaddr_in peer;
    socklen_t peer_len;
    unsigned char rx_buf[128];
    unsigned char tx_buf[128];
};

static const char* xpr_icmp_echo_request_pattern =
    "=== XPR_ICMP ECHO REQEUST ===";

static unsigned short inet_chksum(unsigned char* data, int length)
{
    int nleft = length;
    int sum = 0;
    unsigned short* w = (unsigned short*)data;
    unsigned short answer = 0;

    while (nleft > 1) {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1) {
        *(unsigned char*)(&answer) = *(unsigned char*)w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;

    return answer;
}

static int XPR_ICMP_Init(XPR_ICMP* icmp, const char* dev)
{
    struct timeval tmo = {0, 300000};
    //
    icmp->sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (icmp->sock < 0) {
        return XPR_ERR_ERROR;
    }
    if (setsockopt(icmp->sock, SOL_SOCKET, SO_SNDTIMEO, &tmo, sizeof(tmo)) <
        0) {
        return XPR_ERR_ERROR;
    }
    if (setsockopt(icmp->sock, SOL_SOCKET, SO_RCVTIMEO, &tmo, sizeof(tmo)) <
        0) {
        return XPR_ERR_ERROR;
    }
    return XPR_ERR_OK;
}

static int XPR_ICMP_Fini(XPR_ICMP* icmp)
{
    if (icmp->sock > 0) {
        close(icmp->sock);
        icmp->sock = -1;
    }
    return XPR_ERR_OK;
}

XPR_API XPR_ICMP* XPR_ICMP_New(const char* dev)
{
    XPR_ICMP* icmp = (XPR_ICMP*)calloc(sizeof(*icmp), 1);
    if (icmp) {
        XPR_ICMP_Init(icmp, dev);
    }
    return icmp;
}

XPR_API int XPR_ICMP_Destroy(XPR_ICMP* icmp)
{
    if (icmp) {
        XPR_ICMP_Fini(icmp);
        free(icmp);
    }
    return XPR_ERR_OK;
}

XPR_API int XPR_ICMP_EchoRequest(XPR_ICMP* icmp, const char* host)
{
    int n = 0;
    struct icmp* icmp_pkt = 0;
    struct sockaddr_in to = {0};
    struct timeval* tv = 0;
    //
    memset(icmp->tx_buf, 0, sizeof(icmp->tx_buf));
    //
    icmp_pkt = (struct icmp*)icmp->tx_buf;
    icmp_pkt->icmp_type = ICMP_ECHO;
    icmp_pkt->icmp_code = 0;
    icmp_pkt->icmp_cksum = 0;
    icmp_pkt->icmp_seq = htons(icmp->seq++);
    icmp_pkt->icmp_id = getpid();
    tv = (struct timeval*)icmp_pkt->icmp_data;
    gettimeofday(tv, NULL);
    memcpy(icmp_pkt->icmp_data + 8, xpr_icmp_echo_request_pattern,
           strlen(xpr_icmp_echo_request_pattern));
    icmp_pkt->icmp_cksum =
        inet_chksum((unsigned char*)icmp_pkt, sizeof(icmp->tx_buf));

    memset(&to, 0, sizeof(to));
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = inet_addr(host);

    n = sendto(icmp->sock, icmp->tx_buf, sizeof(icmp->tx_buf), 0,
               (struct sockaddr*)&to, sizeof(to));

    return n > 0 ? XPR_ERR_OK : XPR_ERR_ERROR;
}

XPR_API int XPR_ICMP_Poll(XPR_ICMP* icmp)
{
    int n = 0;
    icmp->peer_len = sizeof(icmp->peer);
    n = recvfrom(icmp->sock, icmp->rx_buf, sizeof(icmp->rx_buf), 0,
                 (struct sockaddr*)&icmp->peer, (socklen_t*)&icmp->peer_len);
    return n > 0 ? XPR_ERR_OK : XPR_ERR_ERROR;
}

static int XPR_ICMP_IsHostResponded(XPR_ICMP* icmp, const char* host)
{
    struct in_addr host_addr = {0};
    inet_pton(AF_INET, host, &host_addr);
    if (icmp->peer.sin_addr.s_addr == host_addr.s_addr)
        return XPR_TRUE;
    return XPR_FALSE;
}

XPR_API int XPR_ICMP_Ping(XPR_ICMP* icmp, const char* host)
{
    int n = 0;
    for (n = 0; n < 3; n++)
        XPR_ICMP_EchoRequest(icmp, host);
    while (1) {
        if (XPR_ICMP_Poll(icmp) < 0)
            break;
        if (XPR_ICMP_IsHostResponded(icmp, host))
            return XPR_ERR_OK;
    }
    return XPR_ERR_ERROR;
}
