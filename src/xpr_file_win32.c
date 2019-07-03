#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_file.h>

XPR_API XPR_File* XPR_FileOpen(const char* fn, const char* mode)
{
    DWORD access = GENERIC_READ;
    DWORD creation = OPEN_EXISTING;
    DWORD append = 0;
    char ch = 0;
    XPR_File* f = 0;
    while (ch = *mode++) {
        switch (ch) {
        case 'a':
            access |= GENERIC_WRITE;
            append = 1;
            break;
        case 'c':
            creation = CREATE_ALWAYS;
            break;
        case 'r':
            access |= GENERIC_READ;
            break;
        case 'w':
            access |= GENERIC_WRITE;
            break;
        case 't':
            creation = TRUNCATE_EXISTING;
            break;
        default:
            break;
        }
    }
    f = (XPR_File*)CreateFileA(fn, access, 0, NULL, creation,
                               FILE_ATTRIBUTE_NORMAL, NULL);
    if (f == INVALID_HANDLE_VALUE)
        f = 0;
    if (f && append)
        SetFilePointer((HANDLE)f, 0, NULL, FILE_END);
    return f;
}

XPR_API int XPR_FileClose(XPR_File* f)
{
    return CloseHandle(f) ? 0 : -1;
}

// FIXME:
XPR_API int XPR_FileFlush(XPR_File* f)
{
    return XPR_ERR_ERROR;
}

XPR_API int XPR_FileRead(XPR_File* f, uint8_t* buffer, int size)
{
    DWORD readed = 0;
    if (!ReadFile((HANDLE)f, buffer, size, &readed, NULL))
        return -1;
    return readed;
}

XPR_API int XPR_FileWrite(XPR_File* f, const uint8_t* data, int length)
{
    DWORD wroted = 0;
    if (!WriteFile((HANDLE)f, data, length, &wroted, NULL))
        return -1;
    return wroted;
}

// FIXME:
XPR_API int64_t XPR_FileSeek(XPR_File* f, int64_t offset, int whence)
{
    if (offset < 0)
        return (int64_t)SetFilePointer((HANDLE)f, 0, NULL, FILE_END) ? 0 : -1;
    return (int64_t)SetFilePointer((HANDLE)f, (LONG)offset, NULL, FILE_BEGIN)
               ? 0
               : -1;
}

// FIXME:
XPR_API int64_t XPR_FileSize(const XPR_File* f)
{
    return 0;
}
