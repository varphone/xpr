#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_arr.h>
#include <xpr/xpr_common.h>
#include <xpr/xpr_errno.h>


struct XPR_ARR
{
    int bitsPerSample;
    int channels;
    int srcSampleRate;
    int dstSampleRate;
};

static unsigned int g_xpr_arr_8k_11k_table[441];

static void g_xpr_arr_8k_11k_table_init(void)
{
    int i = 0;
    unsigned int a = 0;
    for (i = 0; i < 441; i++) {
        a = i * 8000 / 11025;
        g_xpr_arr_8k_11k_table[i] = a > 320 ? 319 : a;
    }
}

XPR_API int XPR_ARR_Init(void)
{
    g_xpr_arr_8k_11k_table_init();
    return 0;
}

XPR_API void XPR_ARR_Fini(void)
{
}

XPR_API XPR_ARR* XPR_ARR_Open(void)
{
    XPR_ARR* r = (XPR_ARR*)calloc(sizeof(*r), 1);
    if (r) {
        r->srcSampleRate = 0;
        r->dstSampleRate = 0;
    }
    return r;
}

XPR_API int XPR_ARR_Close(XPR_ARR* r)
{
    if (r) {
        free((void*)r);
    }
    return 0;
}

XPR_API void XPR_ARR_SetBitsPerSample(XPR_ARR* r, int bps)
{
    r->bitsPerSample = bps;
}

XPR_API void XPR_ARR_SetSampleRates(XPR_ARR* r, int from, int to)
{
    r->srcSampleRate = from;
    r->dstSampleRate = to;
}

XPR_API void XPR_ARR_SetChannels(XPR_ARR* r, int channels)
{
    r->channels = channels;
}

XPR_API int XPR_ARR_GetInputSamples(XPR_ARR* r)
{
    if (r->srcSampleRate == 8000 && r->dstSampleRate == 11025)
        return 320;
    return 0;
}

XPR_API int XPR_ARR_GetOutputSamples(XPR_ARR* r)
{
    if (r->srcSampleRate == 8000 && r->dstSampleRate == 11025)
        return 441;
    return 0;
}

static int XPR_ARR_Transform_s16le(XPR_ARR* r, short* src, short* dst)
{
    int i = 0;
    for (i = 0; i < 441; i++)
        dst[i] = src[g_xpr_arr_8k_11k_table[i]];
    return 441;
}

XPR_API int XPR_ARR_Transform(XPR_ARR* r, void* src, void* dst)
{
    if (r->bitsPerSample == 16)
        return XPR_ARR_Transform_s16le(r, (short*)src, (short*)dst);
    return -1;
}
