/*
 * File: xpr_file.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 文件操作接口
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2014-11-21 12:50:43 Friday, 21 November
 * Last Modified : 2019-07-03 05:16:43 Wednesday, 3 July
 * Modified By   : Varphone Wong <varphone@qq.com>
 * ---------------------------------------------------------------------------
 * Copyright (C) 2012 - 2019 CETC55, Technology Development CO.,LTD.
 * Copyright (C) 2012 - 2019 Varphone Wong, Varphone.com.
 * All rights reserved.
 * ---------------------------------------------------------------------------
 * HISTORY:
 * 2019-07-03   varphone    更新版权信息
 * 2014-11-21   varphone    初始版本建立
 */
#ifndef XPR_FILE_H
#define XPR_FILE_H

#include <stddef.h>
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

#ifndef XPR_FILETYPE_TYPE_DEFINED
#define XPR_FILETYPE_TYPE_DEFINED
typedef enum XPR_FileType
{
    XPR_FILE_TYPE_UNKNOWN, // The file type could not be determined.
    XPR_FILE_TYPE_BLK,     // This is a block device.
    XPR_FILE_TYPE_CHR,     // This is a character device.
    XPR_FILE_TYPE_DIR,     // This is a directory.
    XPR_FILE_TYPE_FIFO,    // This is a named pipe (FIFO).
    XPR_FILE_TYPE_LNK,     // This is a symbolic link.
    XPR_FILE_TYPE_REG,     // This is a regular file.
    XPR_FILE_TYPE_SOCK,    // This is a UNIX domain socket.
} XPR_FileType;
#endif // XPR_FILETYPE_TYPE_DEFINED

#ifndef XPR_FILEINFO_TYPE_DEFINED
#define XPR_FILEINFO_TYPE_DEFINED
typedef struct XPR_FileInfo
{
    const char* name;     // The name of the file without path
    const char* fullname; // The name of the file with path
    const char* path;     // The path of the file without name
    size_t size;          // The total bytes of the file
    XPR_FileType type;    // The type of the file
} XPR_FileInfo;
#endif // XPR_FILEINFO_TYPE_DEFINED

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

/// List all regular files in dir into list
/// @param [in] dir     Directory to listing
/// @param [in] pList   Pointer to receive file list
/// @return Num of files or 0 on error
/// @note Call #XPR_Freev(list) if the retval not used
XPR_API int XPR_FilesInDir(const char* dir, char*** pList);

/// Callback for XPR_FileForEach
/// @param [in] opaque  User context data
/// @param [in] name    The name of the file
/// @param [in] type    The type of the file
/// @return XPR_TRUE for continue, XPR_FALSE break loop
/// @warning Don't cached any thing from the fileInfo,
///          you should copy if want to save the result
typedef int (*XPR_FileForEachFn)(void* opaque, const XPR_FileInfo* fileInfo);

/// List all files in dir into callack
/// @param [in] dir     Directory to listing
/// @param [in] filter  Callback for each file
/// @param [in] opaque  User context data for fn
/// @retval XPR_ERR_OK  Success
/// @retval Others      Error
XPR_API int XPR_FileForEach(const char* dir, XPR_FileForEachFn filter,
                            void* opaque);

#ifdef __cplusplus
}
#endif

#endif // XPR_FILE_H
