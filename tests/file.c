#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_file.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_sys.h>
#include <xpr/xpr_utils.h>

static void test_XPR_FilesInDir()
{
    char** list = NULL;
    int files = XPR_FilesInDir(".", &list);
    for (int i = 0; i < files; i++) {
        printf("[%04d] \"%s\"\n", i, list[i]);
    }
    XPR_Freev((void**)(list));
}

static int forEachFile(void* opaque, const XPR_FileInfo* fileInfo)
{
    printf("[F/%d] '%s', '%s', '%s'\n", fileInfo->type, fileInfo->name,
           fileInfo->fullname, fileInfo->path);
    return XPR_TRUE;
}

static void test_XPR_FileForEach()
{
    XPR_FileForEach(".", forEachFile, NULL);
    XPR_FileForEach("./", forEachFile, NULL);
    XPR_FileForEach("..", forEachFile, NULL);
    XPR_FileForEach("../", forEachFile, NULL);
    XPR_FileForEach("/", forEachFile, NULL);
    XPR_FileForEach("/home", forEachFile, NULL);
    XPR_FileForEach("/home/", forEachFile, NULL);
    XPR_FileForEach("/usr/include", forEachFile, NULL);
    XPR_FileForEach("/usr/include/", forEachFile, NULL);
    XPR_FileForEach("/usr/lib", forEachFile, NULL);
    XPR_FileForEach("/usr/lib/", forEachFile, NULL);
}

static void test_XPR_FileCopy()
{
    printf("test_XPR_FileCopy 1\n");
    XPR_File* f = XPR_FileOpen("test-file-copy.txt", "cwt");
    char buf[1024];
    int n = snprintf(buf, sizeof(buf), "Test, CTS = %ld\n", XPR_SYS_GetCTS());
    XPR_FileWrite(f, (const uint8_t*)(buf), n);
    XPR_FileClose(f);
    printf("test_XPR_FileCopy 2\n");
    XPR_FileCopy("test-file-copy.txt.2", "test-file-copy.txt.3");
    printf("test_XPR_FileCopy 3\n");
    XPR_FileCopy("test-file-copy.txt.1", "test-file-copy.txt.2");
    printf("test_XPR_FileCopy 4\n");
    XPR_FileCopy("test-file-copy.txt", "test-file-copy.txt.1");
    printf("test_XPR_FileCopy 5\n");
}

int main(int argc, char** argv)
{
    test_XPR_FilesInDir();
    test_XPR_FileForEach();
    test_XPR_FileCopy();
    return 0;
}
