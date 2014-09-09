#include <stdio.h>
#include <stdlib.h>

static int comp(const void* a, const void* b)
{
    return strcmp(*(char**)a, *(char**)b);
}

static void dumpStringList(char* sl[], int size)
{
    int i = 0;
    printf("Dump string list: %p, %d\n", sl, size);
    for (; i < size; i++)
        printf("[%4d] %s\n", i, sl[i]);
    printf("\n");
}

static void runTest(void)
{
    char* sl[] = {
        "S80a",
        "S81b",
        "S10c",
        "S10a",
        "S08aaa",
        "S11auto",
    };
    dumpStringList(sl, 6);
    qsort(sl, 6, sizeof(sl[0]), comp);
    dumpStringList(sl, 6);
}

int main(int argc, char** argv)
{
    runTest();
    return 0;
}

