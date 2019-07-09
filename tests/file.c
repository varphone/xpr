#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_file.h>
#include <xpr/xpr_mem.h>
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

int main(int argc, char** argv)
{
    test_XPR_FilesInDir();
    test_XPR_FileForEach();
    return 0;
}
