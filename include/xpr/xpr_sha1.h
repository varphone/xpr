#ifndef XPR_SHA1_H
#define XPR_SHA1_H

/*
   SHA-1 in C
   By Steve Reid <steve@edmweb.com>
   100% Public Domain
 */

#include <stdint.h>
#include <xpr/xpr_common.h>

/// @brief SHA1 哈希值字串缓冲区长度
#define XPR_SHA1_HASH_BUF	42

typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    uint8_t buffer[64];
} XPR_SHA1_CTX;

XPR_API void XPR_SHA1Transform(uint32_t state[5], const uint8_t buffer[64]);

XPR_API void XPR_SHA1Init(XPR_SHA1_CTX* context);

XPR_API void XPR_SHA1Update(XPR_SHA1_CTX* context, const uint8_t *data, size_t length);

XPR_API void XPR_SHA1Final(XPR_SHA1_CTX* context, uint8_t digest[20]);

/// @note `hashOut` must be large than XPR_SHA1_HASH_BUF bytes
XPR_API char* XPR_SHA1Data(const uint8_t* data, size_t length, char* hashOut);

/// @note `hashOut` must be large than XPR_SHA1_HASH_BUF bytes
XPR_API char* XPR_SHA1File(const char* path, char* hashOut);

#endif /* XPR_SHA1_H */