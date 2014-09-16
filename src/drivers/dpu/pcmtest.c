#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <xpr/xpr_dpupriv.h>

typedef struct DriverContext {
    void* owner;
    int exitLoop;              ///< Flag to indicate exit thread loop
    int codec;
    pthread_t workerThread;    ///< Worker thread
} DriverContext;

static int DriverContextInit(DriverContext* dc)
{
    dc->codec = AV_FOURCC_S16L;
    return 0;
}

static int DriverContextFini(DriverContext* dc)
{
    return 0;
}

static int waitForEncoderReady(DriverContext* dc)
{
    return 0;
}

static int loopReadBitstream(DriverContext* dc)
{
    uint8_t data[7056];
    XPR_StreamBlock bsfi = {0};
    bsfi.buffer = data;
    bsfi.bufferSize = 7056;
    bsfi.data = data;
    bsfi.dataSize = 7056;
    bsfi.pts = 0;
    bsfi.flags = XPR_STREAMBLOCK_FLAG_EOF;
    while (!dc->exitLoop) {
        if (XPR_DPU_DeliverStreamBlock(dc->owner, &bsfi) < 0)
            break;
        bsfi.pts += 3600;
        usleep(39000);
    }
    return dc->exitLoop ? 0 : -1;
}

static void* workerThread(void* args)
{
    DriverContext* dc = (DriverContext*)args;
    while (!dc->exitLoop) {
        waitForEncoderReady(dc);
        //
        loopReadBitstream(dc);
    }
    return (void*)0;
}

static int startWorkerThread(DriverContext* dc)
{
    if (pthread_create(&dc->workerThread, NULL, workerThread, dc) < 0) {
        dc->workerThread = 0;
        return -1;
    }
    //enableThreadRealtimeSchedule(dc->workerThread);
    return 0;
}

static int stopWorkerThread(DriverContext* dc)
{
    if (dc->workerThread != 0) {
        dc->exitLoop = 1;
        pthread_join(dc->workerThread, NULL);
        dc->workerThread = 0;
    }

    return 0;
}

// Driver Public Interfaces
////////////////////////////////////////////////////////////////////////////////

static int init(XPR_DPU* ctx)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;
    dc->owner = (void*)ctx;
    return DriverContextInit(dc);
}

static int dein(XPR_DPU* ctx)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;
    return DriverContextFini(dc);
}

static int start(XPR_DPU* ctx)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;
    return startWorkerThread(dc);
}

static int stop(XPR_DPU* ctx)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;
    return stopWorkerThread(dc);
}

static int getStreamCodec(XPR_DPU* ctx, int streamId)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;
    return dc->codec;
}

static int getStreamCount(XPR_DPU* ctx)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;
    return 1;
}

static int waitForReady(XPR_DPU* ctx)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;
    return waitForEncoderReady(dc);
}

// Registry
///////////////////////////////////////////////////////////////////////////////
struct XPR_DPU_Driver xpr_dpu_driver_pcmtest = {
    .name = "pcmtest",
    .description = "PCM testing DPU driver",
    .identifier = XPR_DPU_ID_PCMTEST,
    .privateDataSize = sizeof(struct DriverContext),
    .capabilities = XPR_DPU_CAP_MASK,
    .init = init,
    .dein = dein,
    .start = start,
    .stop = stop,
    .getStreamCodec = getStreamCodec,
    .getStreamCount = getStreamCount,
    .getStreamParam = 0,
    .setStreamParam = 0,
    .waitForReady = waitForReady,
};
