#if !defined(HAVE_XPR_GPIO_DRIVER_A5S)
#include <xpr/xpr_errno.h>
#include <xpr/xpr_gpio.h>

XPR_API int XPR_GPIO_Init(void)
{
    return XPR_ERR_ERROR;
}

XPR_API int XPR_GPIO_Fini(void)
{
    return XPR_ERR_ERROR;
}

XPR_API int XPR_GPIO_Get(int port, int* value)
{
    return XPR_ERR_ERROR;
}

XPR_API int XPR_GPIO_Set(int port, int value)
{
    return XPR_ERR_ERROR;
}

XPR_API int XPR_GPIO_GetMode(int port, int* mode)
{
    return XPR_ERR_ERROR;
}

XPR_API int XPR_GPIO_SetMode(int port, int mode)
{
    return XPR_ERR_ERROR;
}

#endif
