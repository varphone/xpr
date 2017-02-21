#ifndef XPR_ICMP_H
#define XPR_ICMP_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_ICMP_TYPE_DEFINED
#define XPR_ICMP_TYPE_DEFINED
struct XPR_ICMP;
typedef struct XPR_ICMP XPR_ICMP;
#endif // XPR_ICMP_TYPE_DEFINED

XPR_ICMP* XPR_ICMP_New(const char* dev);
int XPR_ICMP_Destroy(XPR_ICMP* arp);
int XPR_ICMP_Ping(XPR_ICMP* arp, const char* host);

#ifdef __cplusplus
}
#endif

#endif // XPR_ICMP_H
