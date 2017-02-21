#ifndef XPR_OEIS_H
#define XPR_OEIS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct XPR_OEIS;
typedef struct XPR_OEIS XPR_OEIS;

typedef enum XPR_OEIS_ERROR {
    XPR_OEIS_ERROR_NONE,
    XPR_OEIS_ERROR_INVARG,  ///< Invalid arguments
    XPR_OEIS_ERROR_FORBID,  ///< Forbidden
    XPR_OEIS_ERROR_SSNTMO,  ///< Session timeout
    XPR_OEIS_ERROR_UNAUTH,  ///< Unauthorized
    XPR_OEIS_ERROR_MAX,
} XPR_OEIS_ERROR;

typedef enum XPR_OEIS_PARAM {
    XPR_OEIS_PARAM_AKEY,
    XPR_OEIS_PARAM_CHUNKED,
    XPR_OEIS_PARAM_PASSWORD,
    XPR_OEIS_PARAM_UID,
    XPR_OEIS_PARAM_USERNAME,
    XPR_OEIS_PARAM_MAX,
} XPR_OEIS_PARAM;

XPR_OEIS* XPR_OEIS_New(const char* url);

int XPR_OEIS_Destroy(XPR_OEIS* oeis);

int XPR_OEIS_SetParam(XPR_OEIS* oeis, XPR_OEIS_PARAM param, const void* data, int length);

int XPR_OEIS_GetParam(XPR_OEIS* oeis, XPR_OEIS_PARAM param, void* buffer, int* size);

int XPR_OEIS_Post(XPR_OEIS* oeis, const char* data, int length);

int XPR_OEIS_Tick(XPR_OEIS* oeis);

#ifdef __cplusplus
}
#endif

#endif // XPR_OEIS_H
