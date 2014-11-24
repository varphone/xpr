#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xpr/xpr_atomic.h>
#include <xpr/xpr_fifo.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_thread.h>
#include <xpr/xpr_utils.h>

static XPR_Atomic testCounter = 0;
static void* PutData(XPR_Thread* t, void* opaque)
{
    char* s = "ABCDEFGH";
    char* p = 0;
    XPR_Fifo* f = (XPR_Fifo*)opaque;
    while (1) {
        p = XPR_StrDup(s);
        if (p && XPR_FifoPutAsAtomic(f, (uintptr_t)p) < 0)
            XPR_Free(p);
        XPR_AtomicInc(&testCounter);
        if ((testCounter % 100000) == 0)
            printf("Test %u00 KPP Passed\n", testCounter / 100000);
    }
}

static void* GetData(XPR_Thread* t, void* opaque)
{
    char* s = "ABCDEFGH";
    char* p = 0;
    XPR_Fifo* f = (XPR_Fifo*)opaque;
    while (1) {
        p = (char*)XPR_FifoGetAsAtomic(f);
        if (p) {
            if (strcmp(p, s) != 0)
                printf("=== FAILED ===\n");
            XPR_Free(p);
        }
    }
}

static void TestFifo(void)
{
    XPR_Fifo* f = 0;
    XPR_Thread* ths[5];
    testCounter = 0;
    f = XPR_FifoCreate(sizeof(char*), 4098);
    ths[0] = XPR_ThreadCreate(PutData, 0, f);
    ths[1] = XPR_ThreadCreate(GetData, 0, f);
    ths[2] = XPR_ThreadCreate(GetData, 0, f);
    ths[3] = XPR_ThreadCreate(GetData, 0, f);
    ths[4] = XPR_ThreadCreate(GetData, 0, f);
    while (1) {
        XPR_ThreadSleep(50000000);
    }
}

int main(int argc, char** argv)
{
    TestFifo();
    return 0;
}