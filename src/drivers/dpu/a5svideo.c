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

typedef struct DriverContext {
    void* owner;
    int exitLoop;              ///< Flag to indicate exit thread loop
    int fAudioClockFrequency;   ///< Audio frequency
    int fIAVFd;                 ///< IAV device file descriptor
    iav_mmap_info_t fMmapInfo;  ///< IAV bitstream map information
    pthread_t workerThread;    ///< Worker thread
} DriverContext;

static int mapIAVBSB(DriverContext* dc)
{
    if (ioctl(dc->fIAVFd, IAV_IOC_MAP_BSB2, &dc->fMmapInfo) < 0)
        return -1;

    return 0;
}

static int DriverContextInit(DriverContext* dc)
{
    if (dc->fIAVFd > 0)
        return -1;

    if ((dc->fIAVFd = open("/dev/iav", O_RDWR, 0)) < 0)
        return -1;

    if (mapIAVBSB(dc) < 0) {
        close(dc->fIAVFd);
        dc->fIAVFd = -1;
        return -1;
    }

    dc->fAudioClockFrequency = 22050;

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

static int iavIsEncoding(int fd)
{
    int states = 0;
    iav_encode_stream_info_ex_t info;

    for (int i = 0; i < 4; i++) {
        info.id = (1 << i);

        if (ioctl(fd, IAV_IOC_GET_ENCODE_STREAM_INFO_EX, &info) < 0)
            return 0;

        if (info.state == IAV_STREAM_STATE_ENCODING)
            states |= (1 << i);
    }

    return states ? 1 : 0;
}

static int iavSetAudioClockFrequency(int fd, int freq)
{
    if (ioctl(fd, IAV_IOC_SET_AUDIO_CLK_FREQ_EX, freq) < 0) {
        return -1;
    }

    return 0;
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
    iavSetAudioClockFrequency(dc->fIAVFd, dc->fAudioClockFrequency);
    iavStartEncodeEx(dc->fIAVFd, 0x0F);
    return 0;
}

static int waitForEncoderReady(DriverContext* dc)
{
    while (!iavIsEncoding(dc->fIAVFd)) {
        fprintf(stderr, "video encoder does not ready ...\n");
        usleep(500000);
    }

    return 0;
}

static int64_t ConvertTS(uint32_t ts)
{
    int64_t v = ts;
    v = v * 1000000 / 90000;
    return v;
}

static int loopReadBitstream(DriverContext* dc)
{
    bits_info_ex_t bsi = {0};
    uint32_t sessions[8] = {0};
    uint32_t lastptss[8] = {0};
    uint32_t step = 0;
    int64_t ptss[8] = {0};
    XD_StreamBlock bsfi = {0};
    int eioOccurs = 0;
    int nopLoops = 0;

    // Loop for read bitstream.
    while (!dc->exitLoop) {
        if (nopLoops > 50) {
            break;
        }

        if (!iavIsEncoding(dc->fIAVFd)) {
            usleep(100 * 1000);
            continue;
        }

        if (ioctl(dc->fIAVFd, IAV_IOC_READ_BITSTREAM_EX, &bsi) < 0) {
            if (errno != EIO)
                break;

            if (eioOccurs++ > 10)
                break;

            nopLoops++;
            continue;
        }

        // Reset EIO occurs counter
        eioOccurs = 0;

        //
        if (bsi.stream_end) {
            nopLoops++;
            continue;
        }

        if (bsi.stream_id < 0 || bsi.stream_id > 3) {
            nopLoops++;
            continue;
        }

        // Reset nop loop counter
        nopLoops = 0;

        // Check the session id.
        if (sessions[bsi.stream_id] != 0 &&
            bsi.session_id != sessions[bsi.stream_id]) {
            DPUDeliverEvent(dc->owner, XD_DPU_EV_FMTCHGD, NULL, 0);
            //bsi.PTS = 0;
            lastptss[bsi.stream_id] = 0;
            ptss[bsi.stream_id] = 0;
            printf("new session: %d\n", bsi.session_id);
        }

        if (ptss[bsi.stream_id] == 0) {
            ptss[bsi.stream_id] = DPUGetCTS(dc->owner, 1000000);
            printf("CTS: %lld\n", ptss[bsi.stream_id]);
        }

        if (lastptss[bsi.stream_id] == 0)
            lastptss[bsi.stream_id] = bsi.PTS;

        if (bsi.PTS > 0) {
            if (bsi.PTS < lastptss[bsi.stream_id])
                step = (1<<30) + lastptss[bsi.stream_id] - bsi.PTS;
            else
                step = bsi.PTS - lastptss[bsi.stream_id];
            lastptss[bsi.stream_id] = bsi.PTS;
            ptss[bsi.stream_id] += ConvertTS(step);
        }

        sessions[bsi.stream_id] = bsi.session_id;
        bsfi.data = (uint8_t*)bsi.start_addr;
        bsfi.dataSize = bsi.pic_size;
        bsfi.flags  = bsi.pic_type;
        bsfi.flags |= XD_STREAMBLOCK_FLAG_EOF;
        bsfi.pts = ptss[bsi.stream_id]; //ConvertTS(bsi.PTS);
        bsfi.dts = ptss[bsi.stream_id]; //bsfi.pts;
        bsfi.track = bsi.stream_id;
        bsfi.codec = (bsi.pic_type >= JPEG_STREAM) ? AV_FOURCC_JPEG : AV_FOURCC_H264;
        // A2 : the start_addr does not plus the mmap address.
        //      so the mmapAddr must be the mmap address.
        // A5S: the start_addr was plus the mmap address.
        //      so the mmapAddr must be 0.
        bsfi.buffer = dc->fMmapInfo.addr;
        bsfi.bufferSize = dc->fMmapInfo.length;

        DPUDeliverStreamBlock(dc->owner, &bsfi);
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
        restartEncoder(dc);
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
    iav_encode_format_ex_t encFormat = {0};
    encFormat.id = 1 << streamId;
    if (ioctl(dc->fIAVFd, IAV_IOC_GET_ENCODE_FORMAT_EX, &encFormat) < 0) {
        return AV_FOURCC_NULL;
    }
    if (encFormat.encode_type == IAV_ENCODE_H264)
        return AV_FOURCC_H264;
    else if (encFormat.encode_type == IAV_ENCODE_MJPEG) {
        return AV_FOURCC_JPEG;
    }
    return AV_FOURCC_NULL;
}

static int getStreamCount(DPUContext* ctx)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;
    return iavGetStreamCount(dc->fIAVFd);
}

static int getStreamParam_qulity(DriverContext* dc, int streamId, void* buffer, int* size)
{
    iav_encode_format_ex_t encFormat = {0};
    iav_jpeg_config_ex_t jpegConf = {0};
    encFormat.id = 1 << streamId;
    if (ioctl(dc->fIAVFd, IAV_IOC_GET_ENCODE_FORMAT_EX, &encFormat) < 0)
        return -1;
    if (encFormat.encode_type == IAV_ENCODE_H264)
        return -1;
    else if (encFormat.encode_type == IAV_ENCODE_MJPEG) {
        jpegConf.id = 1 << streamId;
        if (ioctl(dc->fIAVFd, IAV_IOC_GET_JPEG_CONFIG_EX, &jpegConf) < 0)
            return -1;
        *(int*)buffer = jpegConf.quality;
        *size = 4;
    }
    return 0;
}

static int getStreamParam(DPUContext* ctx, int streamId, const char* param,
                          void* buffer, int* size)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;
    if (strcmp(param, "quality") == 0)
        return getStreamParam_qulity(dc, streamId, buffer, size);
    return -1;
}

static int setStreamParam(DPUContext* ctx, int streamId, const char* param,
                          const void* data, int length)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;
    return -1;
}

static int waitForReady(DPUContext* ctx)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;
    return waitForEncoderReady(dc);
}

// Registry
///////////////////////////////////////////////////////////////////////////////
struct DPUDriver xpr_dpu_driver_a5svideo = {
    .name = "a5svideo",
    .description = "A5S Video Bitstream Read DPU driver",
    .identifier = XPR_DPU_ID_A5SVIDEO,
    .privateDataSize = sizeof(struct DriverContext),
    .capabilities = XPR_DPU_CAP_MASK,
    .init = init,
    .dein = dein,
    .start = start,
    .stop = stop,
    .getStreamCodec = getStreamCodec,
    .getStreamCount = getStreamCount,
    .getStreamParam = getStreamParam,
    .setStreamParam = setStreamParam,
    .waitForReady = waitForReady,
};

#endif // BOARD_MAJOR_A5S

