#include <xpr/xpr_errno.h>
#include <xpr/xpr_devcaps.h>

struct XPR_DevCaps
{
    int version;
};

typedef struct XPR_DevCaps XPR_DevCaps;

static XPR_DevCaps xpr_devcaps = {0};

int XPR_DevCapsInit(void)
{
    return 0;
}

int XPR_DevCapsFini(void)
{
    return 0;
}

int XPR_DevCapsGetInteger(int capId, int devId)
{
    return 0;
}

int64_t XPR_DevCapsGetInt64(int capId, int devId)
{
    return 0;
}

const char* XPR_DevCapsGetString(int capId, int devId)
{
    return 0;
}

const char** XPR_DevCapsGetStrings(int capId, int devId)
{
    return 0;
}

int XPR_DevCapsUpdate(const void* data, int size)
{
    return XPR_ERR_ERROR;
}
