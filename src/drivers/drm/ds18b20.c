#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_drm.h>

// FIXME:
XPR_API int XPR_DRM_Config(int request, const void* data, int size)
{
    return XPR_ERR_ERROR;
}

// FIXME:
XPR_API int XPR_DRM_Init(void)
{
    return XPR_ERR_ERROR;
}

// FIXME:
XPR_API void XPR_DRM_Fini(void)
{
}

// FIXME:
XPR_API int XPR_DRM_Verify(void)
{
    return XPR_ERR_ERROR;
}

// FIXME:
XPR_API int XPR_DRM_GetSerial(uint8_t* buffer, int* length)
{
    return XPR_ERR_ERROR;
}

// FIXME:
XPR_API const char* XPR_DRM_GetSerialString(void)
{
    return XPR_DRM_SERIAL_STRING_NULL;
}

// FIXME:
XPR_API const char* XPR_DRM_GetUuidString(void)
{
    return XPR_DRM_UUID_STRING_NULL;
}

// FIXME:
XPR_API int XPR_DRM_InstallSerial(const uint8_t* data, int length)
{
    return XPR_ERR_ERROR;
}

