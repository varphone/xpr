#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <xpr/xpr_atomic.h>
#include <xpr/xpr_crc.h>
#include <xpr/xpr_errno.h>
#include <xpr/xpr_fq.h>
#include <xpr/xpr_mem.h>
#include <xpr/xpr_thread.h>
#include <xpr/xpr_utils.h>

#if defined(_MSC_VER)
#  if defined(DEBUG) || defined(_DEBUG)
#pragma comment(lib, "libxprd.lib")
#  else
#pragma comment(lib, "libxpr.lib")
#  endif
#endif

struct TestData {
    uint32_t crc;
    char     str[256];
};

static struct TestData testData = {
    0x00000000,
    "ABCDEFGHIJKLMNOP"
    "================"
    "++++++++++++++++"
    "################"
};

static XPR_Atomic testCounter = 0;
static void* PutData(void* opaque, XPR_Thread* t)
{
    int         err = XPR_ERR_ERROR;
    int         len = 0;
    XPR_FQ*     fq  = (XPR_FQ*)opaque;
    uint32_t    crc = 0;
    while (1) {
        len = rand() % 64;
        testData.crc = XPR_CRC32(0, testData.str, len);
        err = XPR_FQ_PushBackRaw(fq, (const void*)&testData, len + 4, rand() % 255);
        if (err == XPR_ERR_OK) {
            XPR_AtomicInc(&testCounter);
            if ((testCounter % 100000) == 0)
                printf("Test %lu00 KPP Passed\n", testCounter / 100000);
        }
    }
    return NULL;
}

static void* GetData(void* opaque, XPR_Thread* t)
{
    XPR_FQ*             fq  = (XPR_FQ*)opaque;
    uint32_t            crc = 0;
    XPR_FQ_ENTRY*       ent = NULL;
    struct TestData*    td  = NULL;
    while (1) {
        ent = XPR_FQ_PopFront(fq);
        if (ent) {
            td = (struct TestData*)(ent->data);
            crc = XPR_CRC32(0, td->str, ent->length - 4);
            if (td->crc != crc)
                printf("=== FAILED <%08X, %08X>, [%.*s] ===\n", td->crc, crc, (int)(ent->length - 4), td->str);
            //else
            //    printf("%08X <%08X, %08X>, [%.*s]\n", ent->flags, td->crc, crc, ent->length - 4, td->str);
            XPR_FQ_ReleaseEntry(fq, ent);
        }
    }
    return NULL;
}

static void TestFQ(void)
{
    XPR_FQ* fq = 0;
    XPR_Thread* ths[5];
    testCounter = 0;
    fq = XPR_FQ_Create(100, 409800);
    ths[0] = XPR_ThreadCreate(PutData, 0, fq);
    ths[1] = XPR_ThreadCreate(GetData, 0, fq);
    ths[2] = XPR_ThreadCreate(GetData, 0, fq);
    ths[3] = XPR_ThreadCreate(GetData, 0, fq);
    ths[4] = XPR_ThreadCreate(GetData, 0, fq);
}

int main(int argc, char** argv)
{
    srand((unsigned)time(0));
    TestFQ();
    system("pause");
    return 0;
}
