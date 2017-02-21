#ifndef XPR_PES_H
#define XPR_PES_H

#include <stdint.h>

#define XPR_PES_HDR_SIZE 40

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_PES_TYPE_DEFINED
#define XPR_PES_TYPE_DEFINED
struct XPR_PES;
typedef struct XPR_PES XPR_PES;
#endif // XPR_PES_TYPE_DEFINED

#ifndef XPR_PES_HEADER_TYPE_DEFINED
#define XPR_PES_HEADER_TYPE_DEFINED
struct XPR_PES_Header;
typedef struct XPR_PES_Header XPR_PES_Header;
#endif // XPR_PES_HEADER_TYPE_DEFINED

#ifndef XPR_PES_WRITECALLBACK_TYPE_DEFINED
#define XPR_PES_WRITECALLBACK_TYPE_DEFINED
typedef int (* XPR_PES_WriteCallback)(const uint8_t* data, int length,
                                      void* opaque);
#endif // XPR_PES_WRITECALLBACK_TYPE_DEFINED

XPR_PES* XPR_PES_Open(const char* url);

int XPR_PES_Close(XPR_PES* p);

int XPR_PES_AddWriteCallback(XPR_PES* f, XPR_PES_WriteCallback cb,
                             void* opaque);

int XPR_PES_WriteHeader(XPR_PES* p, const XPR_PES_Header* hdr);

int XPR_PES_WriteFrame(XPR_PES* p, const uint8_t* data, int length,
                       int codec, int64_t pts);

int XPR_PES_WriteFramePartial(XPR_PES* p, const uint8_t* data, int length,
                              int codec, int64_t pts, int firstPart);

int XPR_PES_WriteTailer(XPR_PES* p);

int XPR_PES_WriteOpaque(XPR_PES* p, const uint8_t* data, int length);

XPR_PES_Header* XPR_PES_HeaderNew(XPR_PES* p);

void XPR_PES_HeaderInit(XPR_PES* p, XPR_PES_Header* hdr);

void XPR_PES_HeaderDestroy(XPR_PES_Header* hdr);

void XPR_PES_HeaderSetAudioPID(XPR_PES_Header* hdr, int flags);

void XPR_PES_HeaderSetVideoPID(XPR_PES_Header* hdr, int flags);

#ifdef __cplusplus
}
#endif

#endif // XPR_PES_H

