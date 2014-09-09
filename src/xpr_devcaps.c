#include <xpr/xpr_devcaps.h>

struct XPR_DEVCAPS
{
    int version;
};

typedef struct XPR_DEVCAPS XPR_DEVCAPS;

static XPR_DEVCAPS xprDevCaps = {0};

int XPR_DEVCAPS_Init(void)
{
    return 0;
}

int XPR_DEVCAPS_Fini(void)
{
    return 0;
}

int XPR_DEVCAPS_GetInteger(int capId, int devId)
{
    return 0;
}

const char* XPR_DEVCAPS_GetString(int capId, int devId)
{
    return 0;
}

const char** XPR_DEVCAPS_GetStrings(int capId, int devId)
{
    return 0;
}

int XPR_DEVCAPS_GetVersion(void)
{
    return xprDevCaps.version;
}

