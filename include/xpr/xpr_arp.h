#ifndef XPR_ARP_H
#define XPR_ARP_H

#include <xpr/xpr_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_ARP_TYPE_DEFINED
#define XPR_ARP_TYPE_DEFINED
struct XPR_ARP;
typedef struct XPR_ARP XPR_ARP;
#endif // XPR_ARP_TYPE_DEFINED

XPR_API XPR_ARP* XPR_ARP_New(const char* dev);

XPR_API int XPR_ARP_Destroy(XPR_ARP* arp);

XPR_API int XPR_ARP_Scan(XPR_ARP* arp, const char* host);

#ifdef __cplusplus
}
#endif

#endif // XPR_ARP_H
