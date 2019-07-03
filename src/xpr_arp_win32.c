#include <xpr/xpr_errno.h>
#include <xpr/xpr_arp.h>

struct XPR_ARP {
    int dummy;
};

XPR_API XPR_ARP* XPR_ARP_New(const char* dev)
{
    return NULL;
}

XPR_API int XPR_ARP_Destroy(XPR_ARP* arp)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}

XPR_API int XPR_ARP_Scan(XPR_ARP* arp, const char* host)
{
    return XPR_ERR_GEN_NOT_SUPPORT;
}
