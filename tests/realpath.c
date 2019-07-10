#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static void testPath(const char* p)
{
    int ar = 0;
    char rp[PATH_MAX] = {0};
    char* rr = NULL;
    rr = realpath(p, rp);
    ar = access(rr, R_OK);
    printf("[%s] -> [%s] [%s]\n", p, rp, ar ? "N" : "Y");
}

static void runTest(void)
{
    int i = 0;
    char* pl[] = {
        "",
        ".",
        "./",
        "..",
        "../",
        "/a/b/c",
        "./a/../cc",
        "/a/../../aa",
        "abc",
        "../../aaa",
        "plugins/m1.so",
    };
    for (; i<sizeof(pl)/sizeof(pl[0]); i++)
        testPath(pl[i]);
}

int main(int argc, char** argv)
{
    runTest();
    return 0;
}

