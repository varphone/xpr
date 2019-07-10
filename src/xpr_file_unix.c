#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_file.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_utils.h>

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

XPR_API int64_t XPR_FileSize(const XPR_File* f)
{
    int fd = (int)(uintptr_t)(f);
    struct stat st;
    if (fstat(fd, &st) < 0)
        return 0;
    return st.st_size;
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
        return 0;
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

static XPR_FileType remapFileType(int type)
{
    switch (type) {
    case DT_BLK:
        return XPR_FILE_TYPE_BLK;
    case DT_CHR:
        return XPR_FILE_TYPE_CHR;
    case DT_DIR:
        return XPR_FILE_TYPE_DIR;
    case DT_FIFO:
        return XPR_FILE_TYPE_FIFO;
    case DT_LNK:
        return XPR_FILE_TYPE_LNK;
    case DT_REG:
        return XPR_FILE_TYPE_REG;
    case DT_SOCK:
        return XPR_FILE_TYPE_SOCK;
    default:
        return XPR_FILE_TYPE_UNKNOWN;
    }
}

XPR_API int XPR_FileForEach(const char* dir, XPR_FileForEachFn filter,
                            void* opaque)
{
    char path[PATH_MAX];
    if (realpath(dir, path) == NULL)
        return XPR_ERR_SYS(errno);
    DIR* dirp = opendir(path);
    if (!dirp)
        return XPR_ERR_SYS(errno);
    int slashEnds = xpr_ends_with(path, "/");
    char fullname[PATH_MAX];
    struct dirent* entry;
    XPR_FileInfo fileInfo;
    while ((entry = readdir(dirp)) != NULL) {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 ||
                strcmp(entry->d_name, "..") == 0)
                continue;
        }
        if (slashEnds)
            snprintf(fullname, sizeof(fullname), "%s%s", path, entry->d_name);
        else
            snprintf(fullname, sizeof(fullname), "%s/%s", path, entry->d_name);
        fileInfo.name = entry->d_name;
        fileInfo.fullname = fullname;
        fileInfo.path = path;
        fileInfo.size = 0;
        fileInfo.type = remapFileType(entry->d_type);
        filter(opaque, &fileInfo);
    }
    closedir(dirp);
    return XPR_ERR_OK;
}

XPR_API int XPR_FileCopy(const char* src, const char* dst)
{
    int fdsrc = open(src, O_RDONLY, 0644);
    if (fdsrc < 0)
        return XPR_ERR_SYS(errno);
    int fddst = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fddst < 0) {
        close(fdsrc);
        return XPR_ERR_SYS(errno);
    }
    char buf[1024];
    while (1) {
        int nr = read(fdsrc, buf, sizeof(buf));
        if (nr <= 0)
            break;
        int nw = write(fddst, buf, nr);
        if (nw <= 0)
            break;
    }
    if (fdsrc > 0)
        close(fdsrc);
    if (fddst > 0)
        close(fddst);
    return XPR_ERR_OK;
}

XPR_API int XPR_FileExists(const char* file)
{
    if (access(file, F_OK) != -1)
        return XPR_TRUE;
    return XPR_FALSE;
}
