#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_atomic.h>
#include <xpr/xpr_fifo.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_thread.h>
#include <xpr/xpr_utils.h>

static void* AutoDestroyThread(void* opaque, XPR_Thread* t)
{
    printf("AutoDestroyThread\n");
    return NULL;
}

static void TestAutoDestroy(void)
{
    int i = 0;
    for (i = 0; i < 100000; i++)
        XPR_ThreadCreateEx(AutoDestroyThread, NULL, XPR_THREAD_FLAG_AUTO_DESTROY, 0, 0);
}

int main(int argc, char** argv)
{
    TestAutoDestroy();
    return 0;
}
