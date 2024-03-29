﻿#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_drm.h>
#include <xpr/xpr_errno.h>

#if !defined(HAVE_XPR_DRM_DRIVER_ALPUC_016) &&                                 \
    !defined(HAVE_XPR_DRM_DRIVER_DS18B20) &&                                   \
    !defined(HAVE_XPR_DRM_DRIVER_RECONBALL)

XPR_API int XPR_DRM_Config(int request, const void* data, int size)
{
    return XPR_ERR_SUCCESS;
}

XPR_API int XPR_DRM_Init(void)
{
    return XPR_ERR_SUCCESS;
}

XPR_API int XPR_DRM_Fini(void)
{
    return XPR_ERR_SUCCESS;
}

XPR_API int XPR_DRM_Verify(void)
{
    return XPR_ERR_SUCCESS;
}

XPR_API int XPR_DRM_GetSerial(uint8_t* buffer, int* length)
{
    memset(buffer, 0, length ? *length : XPR_DRM_SERIAL_SIZE);
    return XPR_ERR_SUCCESS;
}

XPR_API const char* XPR_DRM_GetSerialString(void)
{
    return XPR_DRM_SERIAL_STRING_NULL;
}

XPR_API const char* XPR_DRM_GetUuidString(void)
{
    return XPR_DRM_UUID_STRING_NULL;
}

XPR_API int XPR_DRM_InstallSerial(const uint8_t* data, int length)
{
    return XPR_ERR_SUCCESS;
}

#endif // HAVE_XPR_DRM_DRIVER_XXX
