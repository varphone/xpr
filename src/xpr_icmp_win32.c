#include <xpr/xpr_errno.h>
#include <xpr/xpr_icmp.h>

struct XPR_ICMP
{
    int dummy;
};

XPR_API XPR_ICMP* XPR_ICMP_New(const char* dev)
{
    return NULL;
}

XPR_API int XPR_ICMP_Destroy(XPR_ICMP* icmp)
{
    return XPR_ERR_ERROR;
}

XPR_API int XPR_ICMP_Ping(XPR_ICMP* icmp, const char* host)
{
    return XPR_ERR_ERROR;
}
