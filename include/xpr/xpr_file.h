#ifndef XPR_FILE_H
#define XPR_FILE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef XPR_FILE_TYPE_DEFINED
#define XPR_FILE_TYPE_DEFINED
struct XPR_File;
typedef struct XPR_File XPR_File;
#endif // XPR_FILE_TYPE_DEFINED

#ifndef XPR_FILESEEKWHENCE_TYPE_DEFINED
#define XPR_FILESEEKWHENCE_TYPE_DEFINED
typedef enum XPR_FileSeekWhence {
    XPR_FILE_SEEK_SET = 0,
    XPR_FILE_SEEK_CUR = 1,
    XPR_FILE_SEEK_END = 2,
} XPR_FileSeekWhence;
#endif // XPR_FILESEEKWHENCE_TYPE_DEFINED

/// @brief Open a file
/// @param [in] fn      File name
/// @param [in] mode    Operation mode(s), 'a'=append,'c'=create always,'r'=read,'w'=write,'t'=truncate
///                     Example: osalOpenFile("file.x", "rw");
/// @return the file handle
XPR_File* XPR_FileOpen(const char* fn, const char* mode);

/// @brief Close a opened file
/// @param [in] f       File handle
/// @retval 0   success
/// @retval -1  failure
int XPR_FileClose(XPR_File* f);

int XPR_FileFlush(XPR_File* f);

int XPR_FileRead(XPR_File* f, uint8_t* buffer, int size);

int XPR_FileWrite(XPR_File* f, const uint8_t* data, int length);

int64_t XPR_FileSeek(XPR_File* f, int64_t offset, int whence);

int64_t XPR_FileSize(const XPR_File* f);

#ifdef __cplusplus
}
#endif

#endif // XPR_FILE_H

