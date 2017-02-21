#ifndef XPR_DPU_OPTIONS_H
#define XPR_DPU_OPTIONS_H

#include <xpr/xpr_dpupriv.h>

#ifdef __cplusplus
extern "C" {
#endif

int XPR_DPU_SetDoubleOption(XPR_DPU* ctx, const char* name,
                            double value);
int XPR_DPU_SetFloatOption(XPR_DPU* ctx, const char* name, float value);
int XPR_DPU_SetIntOption(XPR_DPU* ctx, const char* name, int value);
int XPR_DPU_SetInt64Option(XPR_DPU* ctx, const char* name,
                           int64_t value);
int XPR_DPU_SetStringOption(XPR_DPU* ctx, const char* name,
                            const char* value);
void XPR_DPU_SetDefaultOptions(XPR_DPU* ctx);

#ifdef __cplusplus
}
#endif

#endif // XPR_DPU_OPTIONS_H
