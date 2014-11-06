#ifndef XPR_MCDEC_H
#define XPR_MCDEC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int XPR_MCDEC_Config(int option, const void* data, int length);
int XPR_MCDEC_Init(void);
int XPR_MCDEC_Fini(void);

int XPR_MCDEC_PushData(int target, const uint8_t* data, int length);
int XPR_MCDEC_Reset(int target);

#ifdef __cplusplus
}
#endif

#endif // XPR_MCDEC_H
