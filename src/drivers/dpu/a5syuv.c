#if defined(BOARD_MAJOR_A5S)

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <ambarella/a5s/amba_api.h>
#include "DPU.h"
#include "XD_DPU.h"
#include "XD_StreamBlock.h"
#include "XD_SYS.h"

#define trace_line() printf("%s:%d\n", __FILE__, __LINE__)

extern int enableThreadRealtimeSchedule(pthread_t tid);
extern int enableProcessRealTimeSchedule(pid_t pid);

typedef struct DriverContext {
    void* owner;
    int exitLoop;              ///< Flag to indicate exit thread loop
    int interval;
    int intervalCounter;
    int source;                 ///< YUV Source, 0=SENSOR RAW, 1=PREVIEW, 2=2ND BUFFER, 3=4TH BUFFER, 4=ME1 BUFFER1, 5=ME1 BUFFER2
    int fIAVFd;                 ///< IAV device file descriptor
    iav_mmap_info_t fMmapInfo;  ///< IAV bitstream map information
    pthread_t workerThread;    ///< Worker thread
} DriverContext;

static int mapIAVDSP(DriverContext* dc)
{
    if (ioctl(dc->fIAVFd, IAV_IOC_MAP_DSP, &dc->fMmapInfo) < 0)
        return -1;

    return 0;
}

static int DriverContextInit(DriverContext* dc)
{
    if (dc->fIAVFd > 0)
        return -1;

    if ((dc->fIAVFd = open("/dev/iav", O_RDWR, 0)) < 0)
        return -1;

    if (mapIAVDSP(dc) < 0) {
        close(dc->fIAVFd);
        dc->fIAVFd = -1;
        return -1;
    }

    return 0;
}

static int DriverContextFini(DriverContext* dc)
{
    if (dc->fIAVFd > 0) {
        close(dc->fIAVFd);
        dc->fIAVFd = -1;
    }

    return 0;
}

static int iavIsIdle(int fd)
{
    int states = ioctl(fd, IAV_IOC_GET_STATE);
    return states > 0 ? 1 : 0;
}

/// Start encoding (for a5s).
/// @param [in] fd      opened iav driver
/// @param [in] streams stream indentifiers
/// @retval 0   success
/// @retval -1  error
static int iavStartEncodeEx(int fd, int streams)
{
    iav_encode_format_ex_t fmt;
    iav_encode_stream_info_ex_t info;

    for (int i = 0; i < 4; ++i) {
        info.id = (1 << i);
        if (ioctl(fd, IAV_IOC_GET_ENCODE_STREAM_INFO_EX, &info) < 0) {
            return -1;
        }

        if (info.state == IAV_STREAM_STATE_ENCODING) {
            streams &= ~(1 << i);
        }

        fmt.id = (1 << i);
        if (ioctl(fd, IAV_IOC_GET_ENCODE_FORMAT_EX, &fmt) < 0) {
            return -1;
        }

        if (fmt.encode_type == 0) {
            streams &= ~(1 << i);
        }
    }

    if (streams == 0) {
        return 0;
    }
    if (ioctl(fd, IAV_IOC_START_ENCODE_EX, streams) < 0) {
        return -1;
    }

    return 0;
}

/// Stop encoding (for a5s).
/// @param [in] fd      opened iav driver
/// @param [in] streams stream indentifiers
/// @retval 0   success
/// @retval -1  error
static int iavStopEncodeEx(int fd, int streams)
{
    iav_state_info_t info;
    iav_encode_stream_info_ex_t stream_info;
    if (ioctl(fd, IAV_IOC_GET_STATE_INFO, &info) < 0) {
        return -1;
    }

    if (info.state != IAV_STATE_ENCODING) {
        return 0;
    }

    for (int i = 0; i < 4; ++i) {
        stream_info.id = (1 << i);
        if (ioctl(fd, IAV_IOC_GET_ENCODE_STREAM_INFO_EX, &stream_info) < 0) {
            return -1;
        }

        if (stream_info.state != IAV_STREAM_STATE_ENCODING) {
            streams &= ~(1 << i);
        }
    }

    if (streams == 0) {
        return 0;
    }

    if (ioctl(fd, IAV_IOC_STOP_ENCODE_EX, streams) < 0) {
        return -1;
    }

    return 0;
}


static int restartEncoder(DriverContext* dc)
{
    iavStopEncodeEx(dc->fIAVFd, 0xFF);
    iavStartEncodeEx(dc->fIAVFd, 0x0F);
    return 0;
}

static int waitForEncoderReady(DriverContext* dc)
{
    while (iavIsIdle(dc->fIAVFd)) {
        fprintf(stderr, "video encoder does not ready ...\n");
        usleep(500000);
    }
    return 0;
}

static int loopReadBitstream(DriverContext* dc)
{
    iav_preview_info_t prevInfo = {0};
    iav_yuv_buffer_info_ex_t yuvBufferInfo = {0};
    iav_me1_buffer_info_ex_t me1BufferInfo = {0};
    iav_raw_info_t rawInfo = {0};
    XD_AVFrame outFrame = {0};
    int eioOccurs = 0;
    int nopLoops = 0;

    // Loop for read bitstream.
    while (!dc->exitLoop) {
        if (nopLoops > 50) {
            break;
        }

        if (iavIsIdle(dc->fIAVFd)) {
            usleep(100 * 1000);
            continue;
        }

        if (dc->source == 0) {

            if (ioctl(dc->fIAVFd, IAV_IOC_READ_RAW_INFO, &rawInfo) < 0) {
            printf("loopReadBitstream IAV_IOC_READ_RAW_INFO\n");
                perror("ioctl");
                if (errno != EIO)
                    break;
    
                if (eioOccurs++ > 10)
                    break;

                nopLoops++;
                continue;
            }
        }
        else if (dc->source == 1) {
            prevInfo.interval = 0;
            if (ioctl(dc->fIAVFd, IAV_IOC_READ_PREVIEW_INFO, &prevInfo) < 0) {
                perror("ioctl");
                if (errno != EIO)
                    break;
    
                if (eioOccurs++ > 10)
                    break;

                nopLoops++;
                continue;
            }
        }
        else if (dc->source == 2 || dc->source == 3) {
            yuvBufferInfo.source = dc->source == 2 ? 1 : 3; // 1=second buffer, 3=fourth buffer
            if (ioctl(dc->fIAVFd, IAV_IOC_READ_YUV_BUFFER_INFO_EX, &yuvBufferInfo) < 0) {
                if (errno != EIO)
                    break;
    
                if (eioOccurs++ > 10)
                    break;

                nopLoops++;
                continue;
            }
        }
        else if (dc->source == 4 || dc->source == 5) {
            if (ioctl(dc->fIAVFd, IAV_IOC_READ_ME1_BUFFER_INFO_EX, &me1BufferInfo) < 0) {
                if (errno != EIO)
                    break;
    
                if (eioOccurs++ > 10)
                    break;

                nopLoops++;
                continue;
            }
        }
        // Reset EIO occurs counter
        eioOccurs = 0;


        // Reset nop loop counter
        nopLoops = 0;

        if (dc->source == 0) {
            outFrame.datas[0] = rawInfo.raw_addr;
            outFrame.datas[0] = 0;
            outFrame.datas[2] = 0;
            outFrame.pitches[0] = rawInfo.width;
            outFrame.pitches[1] = 0;
            outFrame.pitches[2] = 0;
            outFrame.format = XD_AV_PIX_FMT_BAY10;
            outFrame.width = rawInfo.width;
            outFrame.height = rawInfo.height;
            outFrame.pts++;
        }
        else if (dc->source == 1) {
            outFrame.datas[0] = prevInfo.y_addr;
            outFrame.datas[1] = prevInfo.uv_addr;
            outFrame.datas[2] = 0;
            outFrame.pitches[0] = prevInfo.pitch;
            outFrame.pitches[1] = prevInfo.pitch;
            outFrame.pitches[2] = 0;
            outFrame.format = XD_AV_PIX_FMT_NV16;
            outFrame.width = 0;
            outFrame.height = 0;
            outFrame.pts = prevInfo.PTS;
        }
        else if (dc->source == 2 || dc->source == 3) {
            outFrame.datas[0] = yuvBufferInfo.y_addr;
            outFrame.datas[1] = yuvBufferInfo.uv_addr;
            outFrame.datas[2] = 0;
            outFrame.pitches[0] = yuvBufferInfo.pitch;
            outFrame.pitches[1] = yuvBufferInfo.pitch;
            outFrame.pitches[2] = 0;
            outFrame.format = dc->source == 2 ? XD_AV_PIX_FMT_NV12 : XD_AV_PIX_FMT_NV16;
            outFrame.width = yuvBufferInfo.width;
            outFrame.height = yuvBufferInfo.height;
            outFrame.pts = yuvBufferInfo.PTS;
        }
        else if (dc->source == 4 || dc->source == 5) {
            if (dc->source == 4) {
                outFrame.datas[0] = me1BufferInfo.main_addr;
                outFrame.pitches[0] = me1BufferInfo.main_pitch;
                outFrame.width = me1BufferInfo.main_width;
                outFrame.height = me1BufferInfo.main_height;
            }
            else {
                outFrame.datas[0] = me1BufferInfo.second_addr;
                outFrame.pitches[0] = me1BufferInfo.second_pitch;
                outFrame.width = me1BufferInfo.second_width;
                outFrame.height = me1BufferInfo.second_height;
            }
            outFrame.datas[1] = 0;
            outFrame.datas[2] = 0;
            outFrame.pitches[1] = 0;
            outFrame.pitches[2] = 0;
            outFrame.format = XD_AV_PIX_FMT_Y8;
            outFrame.pts = me1BufferInfo.PTS;
        }

        if (++(dc->intervalCounter) >= dc->interval) {
            DPUDeliverAVFrame(dc->owner, &outFrame);
            dc->intervalCounter = 0;
        }
    }

    return dc->exitLoop ? 0 : -1;
}

static void* workerThread(void* args)
{
    DriverContext* dc = (DriverContext*)args;
    DPUDeliverEvent(dc->owner, XD_DPU_EV_STARTED, NULL, 0);
    while (!dc->exitLoop) {
        waitForEncoderReady(dc);
        // Restart the encoder, flush the buffer.
        //restartEncoder(dc);
        //
        loopReadBitstream(dc);
    }
    DPUDeliverEvent(dc->owner, XD_DPU_EV_STOPPED, NULL, 0);
    return (void*)0;
}


static int iavGetStreamCount(int fd)
{
    int streams = 0;
    iav_encode_stream_info_ex_t info;

    for (int i = 0; i < 4; i++) {
        info.id = (1 << i);

        if (ioctl(fd, IAV_IOC_GET_ENCODE_STREAM_INFO_EX, &info) < 0)
            return 0;

        if (info.state == IAV_STREAM_STATE_ENCODING)
            streams++;
    }

    return streams;
}


static int startWorkerThread(DriverContext* dc)
{
    if (pthread_create(&dc->workerThread, NULL, workerThread, dc) < 0) {
        dc->workerThread = 0;
        return -1;
    }
    XD_SYS_EnableThreadRealtimeSchedule(dc->workerThread);
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

static int init(DPUContext* ctx)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;
    dc->owner = (void*)ctx;
    return DriverContextInit(dc);
}

static int dein(DPUContext* ctx)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;
    return DriverContextFini(dc);
}

static int start(DPUContext* ctx)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;
    return startWorkerThread(dc);
}

static int stop(DPUContext* ctx)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;
    return stopWorkerThread(dc);
}

static int getStreamCodec(DPUContext* ctx, int streamId)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;
    return AV_FOURCC_NULL;
}

static int getStreamCount(DPUContext* ctx)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;
    return iavGetStreamCount(dc->fIAVFd);
}

static int waitForReady(DPUContext* ctx)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;
    return waitForEncoderReady(dc);
}

// Options
#define OFFSET(x) offsetof(DriverContext, x)
static DPUOption options[] = {
    { "interval", NULL, OFFSET(interval), DPU_OPT_INT, { .i = 1 }, 0, 120, 0, 0 },
    { "source", NULL, OFFSET(source), DPU_OPT_INT, { .i = 4 }, 0, 5, 0, 0 },
    { NULL },
};

// Registry
///////////////////////////////////////////////////////////////////////////////
struct DPUDriver xd_a5syuv_dpu_driver = {
    .name = "a5syuv",
    .description = "A5S YUV Frame Read DPU driver",
    .identifier = DPU_ID_A5SYUV,
    .privateDataSize = sizeof(struct DriverContext),
    .capabilities = DPU_CAP_MASK,
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

#endif // BOARD_MAJOR_A5S

