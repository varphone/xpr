#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_file.h>

static void unframed(const char* src, const char* dst)
{
    XPR_File* f1 = XPR_FileOpen(src, "rb");
    if (XPR_FIEL_IS_NULL(f1)) {
        fprintf(stderr, "Open [%s] failed, errno: %d\n", src, errno);
        abort();
    }

    XPR_File* f2 = XPR_FileOpen(dst, "cwb");
    if (XPR_FIEL_IS_NULL(f2)) {
        fprintf(stderr, "Open [%s] failed, errno: %d\n", dst, errno);
        abort();
    }

    char* buffer = malloc(1024 * 1024);

    while (1) {
        // Read frame size
        uint32_t frameSize = 0;
        int n = XPR_FileRead(f1, (uint8_t*)&frameSize, sizeof(frameSize));
        if (n != sizeof(frameSize))
            break;

        // Check frame size
        if (frameSize <= 0)
            break;

        // Read frame content
        n = XPR_FileRead(f1, buffer, frameSize);
        if (n != frameSize)
            break;

        // Write to new file
        XPR_FileWrite(f2, buffer, frameSize);
    }

    XPR_FileClose(f1);
    XPR_FileClose(f2);
}

int main(int argc, char** argv)
{
    char framedFile[256];
    char unframedFile[256];

    if (argc < 2) {
        printf("Usage: %s <framed file> [unframed file]\n", argv[0]);
        exit(-1);
    }

    strcpy(framedFile, argv[1]);

    if (argc > 2)
        strcpy(unframedFile, argv[2]);
    else
        sprintf(unframedFile, "%s.unframed", framedFile);

    unframed(framedFile, unframedFile);

    return 0;
}