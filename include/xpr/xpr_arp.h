#ifndef XPR_ARP_H
#define XPR_ARP_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_ARP_TYPE_DEFINED
#define XPR_ARP_TYPE_DEFINED
struct XPR_ARP;
typedef struct XPR_ARP XPR_ARP;
#endif // XPR_ARP_TYPE_DEFINED

XPR_ARP* XPR_ARP_New(const char* dev);
int XPR_ARP_Destroy(XPR_ARP* arp);
int XPR_ARP_Scan(XPR_ARP* arp, const char* host);

#ifdef __cplusplus
}
#endif

#endif // XPR_ARP_H
