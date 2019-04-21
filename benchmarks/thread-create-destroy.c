#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_thread.h>
#include <xpr/xpr_utils.h>

static void* run(void* opaque, XPR_Thread* t)
{
    return NULL;
}

void benchmark(void)
{
    int i = 0;
    uint32_t n = 0;
    XPR_Thread* threads[100];
    while (1) {
        for (i = 0; i < 100; i++)
            threads[i] = XPR_ThreadCreate(run, 0, NULL);
        XPR_ThreadSleep(1000000);
        for (i = 0; i < 100; i++) {
            XPR_ThreadJoin(threads[i]);
            XPR_ThreadDestroy(threads[i]);
        }
        n++;
        printf("Benchmark %u times\n", n);
    }
}
