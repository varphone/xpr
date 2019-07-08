#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_file.h>
#include <xpr/xpr_mem.h>

XPR_API XPR_File* XPR_FileOpen(const char* fn, const char* mode)
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
    return (void*)(long)open(fn, modeFlags, 0644);
}

XPR_API int XPR_FileClose(XPR_File* f)
{
    return close((int)(long)f);
}

// FIXME:
XPR_API int XPR_FileFlush(XPR_File* f)
{
    return 0;
}

XPR_API int XPR_FileRead(XPR_File* f, uint8_t* buffer, int size)
{
    return read((int)(long)f, buffer, size);
}

XPR_API int XPR_FileWrite(XPR_File* f, const uint8_t* data, int length)
{
    return write((int)(long)f, data, length);
}

XPR_API int64_t XPR_FileSeek(XPR_File* f, int64_t offset, int whence)
{
    return lseek((int)(long)f, offset, whence);
}

// FIXME:
XPR_API int64_t XPR_FileSize(const XPR_File* f)
{
    return 0;
}

static int countFilesInDir(const char* dir)
{
    DIR* dirp = opendir(dir);
    if (dirp == NULL)
        return 0;
    int files = 0;
    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_REG) {
            files++;
        }
    }
    closedir(dirp);
    return files;
}

XPR_API int XPR_FilesInDir(const char* dir, char*** pList)
{
    int files = countFilesInDir(dir);
    if (files <= 0)
        return NULL;
    char** list = XPR_Alloc(sizeof(char*)*(files+1));
    int index = 0;
    char path[PATH_MAX];
    DIR* dirp = opendir(dir);
    struct dirent* entry;
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_REG) {
            snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);
            list[index] = XPR_StrDup(path);
            index++;
        }
    }
    closedir(dirp);
    list[files] = NULL;
    *pList = list;
    return files;
}