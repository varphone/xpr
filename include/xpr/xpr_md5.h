/* Because MD5 may not be implemented (at least, with the same
 * interface) on all systems, we have our own copy here.
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

#ifndef XPR_MD5_H
#define XPR_MD5_H

#include <stddef.h>
#include <stdint.h>
#include <xpr/xpr_common.h>

/* Definitions of _ANSI_ARGS and EXTERN that will work in either
   C or C++ code:
 */
#undef _ANSI_ARGS_
#if ((defined(__STDC__) || defined(SABER)) && !defined(NO_PROTOTYPE)) || defined(__cplusplus) || defined(USE_PROTOTYPE)
#   define _ANSI_ARGS_(x)       x
#else
#   define _ANSI_ARGS_(x)       ()
#endif

#ifdef __cplusplus
extern "C" {
#endif

/// @brief MD5 哈希值字串缓冲区长度
#define XPR_MD5_HASH_BUF	34

/* MD5 context. */
#ifndef XPR_MD5CONTEXT_TYPE_DEFINED
#define XPR_MD5CONTEXT_TYPE_DEFINED
struct XPR_MD5Context {
	uint32_t state[4];	/* state (ABCD) */
	uint32_t count[2];	/* number of bits, modulo 2^64 (lsb first) */
	uint8_t buffer[64];	/* input buffer */
};
typedef struct XPR_MD5Context XPR_MD5Context;
#endif // XPR_MD5CONTEXT_TYPE_DEFINED

XPR_API void XPR_MD5Init(XPR_MD5Context* ctx);
XPR_API void XPR_MD5Update(XPR_MD5Context* ctx, const uint8_t* input, size_t inputLength);
XPR_API void XPR_MD5Pad(XPR_MD5Context* ctx);
XPR_API void XPR_MD5Final(uint8_t digest[16], XPR_MD5Context* ctx);
XPR_API char* XPR_MD5End(XPR_MD5Context* ctx, char* buf);

/// @brief Do MD5 on file
/// @param [in] path			File path
/// @param [in,out] hashOut		String buffer to receive the md5 string, must be large than XPR_MD5_HASH_BUF bytes
/// @return hashOut on success, NULL on failure
XPR_API char* XPR_MD5File(const char* path, char* hashOut);

/// @brief Do MD5 on memory buffer
/// @param [in] data			Data buffer
/// @param [in] dataLength		Bytes of the data buffer
/// @param [in,out] hashOut		String buffer to receive the md5 string, must be large than XPR_MD5_HASH_BUF bytes
/// @return hashOut on success, NULL on failure
XPR_API char* XPR_MD5Data(const uint8_t* data, size_t dataLength, char* hashOut);

#ifdef __cplusplus
}
#endif

#endif /* XPR_MD5_H */

