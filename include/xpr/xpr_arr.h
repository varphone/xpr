#ifndef XPR_ARR_H
#define XPR_ARR_H

#include <stdint.h>

/// @defgroup xprarr 音频重采样
/// @brief     转换音频采样率
/// @author    Varphone Wong [varphone@163.com]
/// @version   1.0.0
/// @date      2013/4/1
///
/// @{
///

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_ARR_TYPE_DEFINED
#define XPR_ARR_TYPE_DEFINED
struct XPR_ARR;
typedef struct XPR_ARR XPR_ARR;
#endif // XPR_ARR_TYPE_DEFINED

int XPR_ARR_Init(void);

void XPR_ARR_Fini(void);

XPR_ARR* XPR_ARR_Open(void);

int XPR_ARR_Close(XPR_ARR* r);

void XPR_ARR_SetBitsPerSample(XPR_ARR* r, int bps);

void XPR_ARR_SetSampleRates(XPR_ARR* r, int from, int to);

void XPR_ARR_SetChannels(XPR_ARR* r, int channels);

int XPR_ARR_GetInputSamples(XPR_ARR* r);

int XPR_ARR_GetOutputSamples(XPR_ARR* r);

int XPR_ARR_Transform(XPR_ARR* r, void* src, void* dst);

#ifdef __cplusplus
}
#endif

/// @}
///

#endif // XPR_ARR_H

