#ifndef XPR_FILE_H
#define XPR_FILE_H

#include <stdint.h>
#include <xpr/xpr_common.h>

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

#if defined(__unix__) || defined(_linux_)
#define XPR_FIEL_IS_NULL(x)     (((int)(intptr_t)(x)) < 0)
#elif defined(WIN32) || defined(WIN64)
#define XPR_FIEL_IS_NULL(x)     ((x) == 0)
#else
#define XPR_FIEL_IS_NULL(x)     (x)
#endif

/// @brief Open a file
/// @param [in] fn      File name
/// @param [in] mode    Operation mode(s), 'a'=append,'c'=create always,'r'=read,'w'=write,'t'=truncate
///                     Example: osalOpenFile("file.x", "rw");
/// @return the file handle
XPR_API XPR_File* XPR_FileOpen(const char* fn, const char* mode);

/// @brief Close a opened file
/// @param [in] f       File handle
/// @retval 0   success
/// @retval -1  failure
XPR_API int XPR_FileClose(XPR_File* f);

/// @brief 立即刷写到磁盘
/// @retval XPR_ERR_OK
/// @retval XPR_ERR_ERROR
XPR_API int XPR_FileFlush(XPR_File* f);

/// @brief 读取文件内容到内存
/// @retval -1	读取失败
/// @retval  0	文件为空
/// @retval >0	读取到的字节数
XPR_API int XPR_FileRead(XPR_File* f, uint8_t* buffer, int size);

XPR_API int XPR_FileWrite(XPR_File* f, const uint8_t* data, int length);

XPR_API int64_t XPR_FileSeek(XPR_File* f, int64_t offset, int whence);

/// @brief 获取文件大小
/// @return 文件字节数
XPR_API int64_t XPR_FileSize(const XPR_File* f);

#ifdef __cplusplus
}
#endif

#endif // XPR_FILE_H

