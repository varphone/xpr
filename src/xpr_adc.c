#if !defined(XPR_ADC_DRIVER_A5S)
#include <xpr/xpr_errno.h>
#include <xpr/xpr_common.h>
#include <xpr/xpr_adc.h>

int XPR_ADC_Config(int cfg, const void* data, int length)
{
    return XPR_ERR_ERROR;
}

int XPR_ADC_Init(void)
{
    //FIXME:
    return XPR_ERR_ERROR;
}

int XPR_ADC_Fini(void)
{
    //FIXME:
    return XPR_ERR_ERROR;
}

int XPR_ADC_IsPortReady(int port)
{
    return XPR_FALSE;
}

int XPR_ADC_IsPortValid(int port)
{
    return XPR_FALSE;
}

int XPR_ADC_GetValue(int port, void* buffer, int* size)
{
    //FIXME:
    return XPR_ERR_ERROR;
}

int XPR_ADC_SetParam(int port, const void* data, int length)
{
    return XPR_ERR_ERROR;
}

int XPR_ADC_GetParam(int port, void* buffer, int* size)
{
    return XPR_ERR_ERROR;
}
#endif

