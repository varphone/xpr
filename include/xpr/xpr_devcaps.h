#ifndef XPR_DEVCAPS_H
#define XPR_DEVCAPS_H

#ifdef __cplusplus
extern "C"
{
#endif

int XPR_DEVCAPS_Init(void);
int XPR_DEVCAPS_Fini(void);
int XPR_DEVCAPS_GetInteger(int capId, int devId);
const char* XPR_DEVCAPS_GetString(int capId, int devId);
const char** XPR_DEVCAPS_GetStringList(int capId, int devId);
int XPR_DEVCAPS_GetVersion(void);

#ifdef __cplusplus
}
#endif

#endif // XPR_DEVCAPS_H

