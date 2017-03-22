#include <xpr/xpr_errno.h>
#include <xpr/xpr_icmp.h>

struct XPR_ICMP {
    int dummy;
};

XPR_ICMP* XPR_ICMP_New(const char* dev)
{
    return NULL;
}

int XPR_ICMP_Destroy(XPR_ICMP* icmp)
{
    return XPR_ERR_ERROR;
}

int XPR_ICMP_Ping(XPR_ICMP* icmp, const char* host)
{
    return XPR_ERR_ERROR;
}
