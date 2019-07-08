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

int main(int argc, char** argv)
{
    test_XPR_FilesInDir();
    return 0;
}
