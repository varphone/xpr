#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <xpr/xpr_dpupriv.h>

typedef struct MDTPacket {
    uint8_t mark;
    uint8_t version;
    uint16_t payload_length;
    uint8_t cols;
    uint8_t rows;
    uint16_t sequence;
    int64_t PTS;
    int64_t CTS;
    uint8_t payload[0];
} MDTPacket;

typedef struct DriverContext {
    void* owner;
    int codec;
    int cols;
    int rows;
    int source;
    int firstFrame;
    XPR_DPU* yuvSource;
    XPR_StreamBlock streamBlock;
    uint32_t* sums[2];
    uint32_t* currSum;
    uint32_t* lastSum;
} DriverContext;


static void ProcessNV12Frame(DriverContext* dc, const XPR_AVFrame* frame)
{
    MDTPacket* mdtp = (MDTPacket*)dc->streamBlock.data;
    // Process the Data
    // ...
    // Update CTS
    mdtp->CTS = XPR_DPU_GetSystemCTS(1000000);
    // Post Out
    XPR_DPU_DeliverStreamBlock(dc->owner, &dc->streamBlock);
}

//calculation the blocks y sum
/// @param [in] dc
/// @param [in] frame
/// @param [in] col         Column index
/// @param [in] row         Row index
static int SumBlocks(DriverContext* dc, const XPR_AVFrame* frame, int col, int row)
{
    int x, y;
    int cx, cy;
    int sum = 0;
    int i = 0;
    int j = 0;
    cx = frame->width / dc->cols;// col number
    cy = frame->height / dc->rows;//row number
    for (i=0; i<cy; i++) {
        x = ((row * cy)) + i * frame->width;//index
        x += cx * col;
        for (j=0; j<cx; j++)
            sum += frame->datas[0][x+j];// data sum
    }
    return sum / cx / cy;
}

static void ProcessY8Frame(DriverContext* dc, const XPR_AVFrame* frame,void* opaque)
{
    int i = 0;
    int j = 0;
    MDTPacket* mdtp = (MDTPacket*)dc->streamBlock.data;
    // Setup packet fixed fields
    mdtp->mark = '$';
    mdtp->version = 0;
    mdtp->payload_length = dc->cols * dc->rows;
    mdtp->cols = dc->cols;
    mdtp->rows = dc->rows;
    mdtp->sequence++;
    // Process the Data
    for (i=0; i<dc->rows; i++) {
        for (j=0;j<dc->cols; j++) {
            dc->currSum[i*dc->cols+j] = SumBlocks(dc, frame, j, i);// sumBlock
        }
    }
    
    if (dc->firstFrame) {
        memcpy(dc->lastSum, dc->currSum, dc->cols * dc->rows * sizeof(dc->lastSum[0]));
        dc->firstFrame = 0;
    } else {
        for (i=0; i<dc->cols*dc->rows; i++) {
            mdtp->payload[i] = abs(dc->currSum[i] - dc->lastSum[i]);// frame differ alg
                    }
        //add framediff mse if accury not enough
        // Update PTS
        mdtp->PTS = frame->pts * 1000000 / 90000;
        dc->streamBlock.pts = mdtp->PTS;
        // Update CTS
        mdtp->CTS = XPR_DPU_GetSystemCTS(1000000);
        dc->streamBlock.dts = mdtp->CTS; 
        // Post Out
        XPR_DPU_DeliverStreamBlock(dc->owner, &dc->streamBlock);
    }
}

static int avframeCallback(XPR_DPU* dpu,
                           const XPR_AVFrame* frame,
                           void* opaque)
{
    DriverContext* dc = (DriverContext*)opaque;
    if (frame->format == XPR_AV_PIX_FMT_NV12)
        ProcessNV12Frame(dc, frame);
    else if (frame->format == XPR_AV_PIX_FMT_Y8)
        ProcessY8Frame(dc, frame, opaque);
    return 0;
}

static int eventCallback(XPR_DPU* dpu, int event, const void* eventData,
                         int eventDataSize, void* opaque)
{
   // printf("Event: %p, %d, %p, %d, %p\n", dpu, event, eventData, eventDataSize, opaque);
    return 0;
}

static int DriverContextInit(DriverContext* dc)
{
    dc->yuvSource = XPR_DPU_Open("{\"driver\":\"a5syuv\"}");
    if (dc->yuvSource) {
        XPR_DPU_SetOption(dc->yuvSource, "source", (const void*)dc->source, 0);
        XPR_DPU_AddAVFrameCallback(dc->yuvSource, avframeCallback, (void*)dc);
        XPR_DPU_AddEventCallback(dc->yuvSource, eventCallback, (void*)dc);
    }
    return 0;
}

static int DriverContextFini(DriverContext* dc)
{
    if (dc->yuvSource) {
        XPR_DPU_Close(dc->yuvSource);
        dc->yuvSource = 0;
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
    dc->firstFrame = 1;
    dc->streamBlock.bufferSize = sizeof(MDTPacket) + dc->cols*dc->rows;
    dc->streamBlock.buffer = malloc(dc->streamBlock.bufferSize);
    dc->streamBlock.data = dc->streamBlock.buffer;
    dc->streamBlock.dataSize = dc->streamBlock.bufferSize;
    dc->streamBlock.codec = AV_FOURCC_MDSD;
    dc->streamBlock.flags = XPR_STREAMBLOCK_FLAG_EOF;
    dc->sums[0] = malloc(128*128*4);
    dc->sums[1] = malloc(128*128*4);
    dc->currSum = dc->sums[0];
    dc->lastSum = dc->sums[1];
    return XPR_DPU_Start(dc->yuvSource);
}

static int stop(XPR_DPU* ctx)
{
    int result = -1;
    DriverContext* dc = (DriverContext*)ctx->privateData;
    result = XPR_DPU_Stop(dc->yuvSource);
    if (dc->streamBlock.buffer) {
        free((void*)dc->streamBlock.buffer);
        dc->streamBlock.buffer = 0;
        dc->streamBlock.bufferSize = 0;
        dc->streamBlock.data = 0;
        dc->streamBlock.dataSize = 0;
        dc->streamBlock.codec = AV_FOURCC_NULL;
    }
    if (dc->sums[0]) {
        free(dc->sums[0]);
        dc->sums[0] = 0;
    }
    if (dc->sums[1]) {
        free(dc->sums[1]);
        dc->sums[1] = 0;
    }
    return result;
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
    return XPR_DPU_WaitForReady(dc->yuvSource);
}

// Options
#define OFFSET(x) offsetof(DriverContext, x)
static XPR_DPU_Option options[] = {
    { "codec", NULL, OFFSET(codec), XPR_DPU_OPT_INT, { .i = AV_FOURCC_MDSD }, 0, 0xffffffff, 0, 0 },
    { "cols", NULL, OFFSET(cols), XPR_DPU_OPT_INT, { .i = 12 }, 1, 128, 0, 0 },
    { "rows", NULL, OFFSET(rows), XPR_DPU_OPT_INT, { .i = 8 }, 1, 128, 0, 0 },
    { "source", NULL, OFFSET(source), XPR_DPU_OPT_INT, { .i = 4 }, 0, 5, 0, 0 },
    { NULL },
};

// Registry
///////////////////////////////////////////////////////////////////////////////
struct XPR_DPU_Driver xpr_dpu_driver_mdsd = {
    .name = "mdsd",
    .description = "Motion Detection Summarily Data DPU driver",
    .identifier = XPR_DPU_ID_MDSD,
    .privateDataSize = sizeof(struct DriverContext),
    .capabilities = XPR_DPU_CAP_MASK,
    .options = options,
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

