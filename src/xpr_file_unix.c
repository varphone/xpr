#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_file.h>

XPR_File* XPR_FileOpen(const char* fn, const char* mode)
{
    int modeFlags = 0;
    char ch = 0;
    while ((ch = *mode++) != 0) {
        switch (ch) {
        case 'a':
            modeFlags |= O_APPEND;
            break;
        case 'c':
            modeFlags |= O_CREAT;
            break;
        case 'r':
            modeFlags |= O_RDONLY;
            break;
        case 'w':
            modeFlags |= O_WRONLY;
            break;
        case 't':
            modeFlags |= O_TRUNC;
            break;
        default:
            break;
        }
    }
    return (void*)open(fn, modeFlags, 0644);
}

int XPR_FileClose(XPR_File* f)
{
    return close((int)f);
}

// FIXME:
int XPR_FileFlush(XPR_File* f)
{
    return 0;
}

int XPR_FileRead(XPR_File* f, uint8_t* buffer, int size)
{
    return read((int)f, buffer, size);
}

int XPR_FileWrite(XPR_File* f, const uint8_t* data, int length)
{
    return write((int)f, data, length);
}

int64_t XPR_FileSeek(XPR_File* f, int64_t offset, int whence)
{
    return lseek((int)f, offset, whence);
}

// FIXME:
int64_t XPR_FileSize(const XPR_File* f)
{
    return 0;
}

