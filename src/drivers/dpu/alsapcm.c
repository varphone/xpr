#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <time.h>
#include <alsa/asoundlib.h>
#include <xpr/xpr_dpu.h>
#include <xpr/xpr_dpupriv.h>
#include <xpr/xpr_sys.h>

#define PCM_SOFT_RESAMPLE 0
#define PCM_LATENCY       100000
#define MAX_DATA_CALLBACKS 16

enum {
	VUMETER_NONE,
	VUMETER_MONO,
	VUMETER_STEREO
};

typedef struct DriverContext {
    void* owner;
    int exitLoop;              ///< Flag to indicate exit thread loop
    pthread_t workerThread;    ///< Worker thread
    snd_pcm_t* pcmHandle;
    int codec;

    int availMin;
    unsigned int format;
    unsigned int bitsPerFrame;
    unsigned int bitsPerSample; ///< 每样本位数, 默认 16
    unsigned int sampleRate;  ///< 采样频率, 默认: 44100
    unsigned int channels;    ///< 声道数
    unsigned int chunkSize;
    unsigned int chunkBytes;
    unsigned int periodFrames;
    unsigned int periodSize;  ///< 每次采集样本数量
    unsigned int periodTime;  ///< 每次采集时间周期
    unsigned int bufferFrames;
    unsigned int bufferSize;  ///< 缓存样本数量
    unsigned int bufferTime;  ///< 缓存时间长度
    int startDelay;
    unsigned int startThreshold;
    int stopDelay;
    unsigned int stopThreshold;

    int canPause;
    int interleaved;
    int latency;     ///< 延时
    int mmap;
    int monotonic;
    int softResample;         ///< 是否使用软件重采样
    int verbose;
    int volume;               ///< 音量
    int vumeter;

    int64_t deltaFrames;
    int64_t deltaDTS;

    snd_output_t* log;
    uint8_t* audioBuffer;

} DriverContext;

static int DriverContextInit(DriverContext* dc)
{
    dc->audioBuffer = malloc(16*1024);
    snd_output_stdio_attach(&dc->log, stdout, 0);
    return 0;
}

static int DriverContextFini(DriverContext* dc)
{
    if (dc->audioBuffer) {
        free(dc->audioBuffer);
        dc->audioBuffer = 0;
    }
    return 0;
}

static int isReadable(DriverContext* dc)
{
    snd_pcm_state_t state = snd_pcm_state(dc->pcmHandle);

    if (state == SND_PCM_STATE_XRUN) {
        printf("SND_PCM_STATE_XRUN\n");
        snd_pcm_prepare(dc->pcmHandle);
    }

    return 1;
}

static int computeBytes(DriverContext* dc, int samples)
{
    return samples * dc->channels * dc->bitsPerSample / 8;
}

static int64_t computeDTS(DriverContext* dc, int samples)
{
    int differ = 0;
    if (dc->sampleRate == 22050)
        differ = 9;
    else if (dc->sampleRate == 44100)
        differ = 4;
    return (int64_t)samples * 90000 / dc->sampleRate + differ;
}

static int readFrame(DriverContext* dc, uint8_t* buffer, int size, int64_t* dts)
{
    int err = 0;
    int samples = dc->periodSize;
    int bytes = computeBytes(dc, samples);

    if (bytes > size) {
        errno = EINVAL;
        return -1;
    }

    samples = snd_pcm_readi(dc->pcmHandle, buffer, samples);

    if (samples < 0) {
        fprintf(stderr, "snd_pcm_readi() failure, error: %s\n", snd_strerror(samples));
        err = snd_pcm_recover(dc->pcmHandle, err, 0);

        if (samples < 0) {
            fprintf(stderr, "snd_pcm_recover() failure, error: %s\n", snd_strerror(samples));
            return -1;
        }

        samples = 0;
    }
    else if (samples > 0) {
        bytes = computeBytes(dc, samples);
        *dts = computeDTS(dc, samples);
        dc->deltaFrames += samples;
        dc->deltaDTS += *dts;
    }
    else {
        bytes = 0;
        *dts  = 0;
    }

    return bytes;
}

static int64_t fixDTS(DriverContext* dc)
{
    int64_t targetDTS = 0, targetDiff = 0;
    if (dc->deltaFrames >= dc->sampleRate * 10) {
        targetDTS = computeDTS(dc, dc->deltaFrames);
        targetDiff = targetDTS - dc->deltaDTS;
        dc->deltaFrames = 0;
        dc->deltaDTS = 0;
    }
    return targetDiff;
}

static int syncPTS(DriverContext* dc)
{
    return 0;
}

static int loopReadBitstream(DriverContext* dc)
{
    uint8_t* buffer = malloc(16 * 1024);
    uint8_t* dummyBuffer = malloc(16 * 1024);
    XPR_StreamBlock bsfi = {0};
    XPR_StreamBlock dummyFrame = {0};
    bsfi.buffer = buffer;
    bsfi.data = buffer;
    bsfi.pts = XPR_DPU_GetCTS(dc->owner, 90000);
    bsfi.flags = XPR_STREAMBLOCK_FLAG_EOF;
    //bsfi.mmapAddr = buffer;
    //bsfi.mmapSize = 16 * 1024;

    dummyFrame.buffer = dummyBuffer;
    dummyFrame.data = dummyBuffer;
    dummyFrame.pts = 0;
    dummyFrame.flags = XPR_STREAMBLOCK_FLAG_EOF;
    //dummyFrame.mmapAddr = dummyBuffer;
    //dummyFrame.mmapSize = 16 * 1024;

    int diffCount = 0;
    int64_t cts = XPR_DPU_GetCTS(dc->owner, 90000);
    int64_t dts = 0;
    while (!dc->exitLoop) {
        if (!isReadable(dc))
            break;

        bsfi.dataSize = readFrame(dc, buffer, bsfi.bufferSize, &dts);

        cts = XPR_DPU_GetCTS(dc->owner, 90000);
        int64_t diff = cts - bsfi.pts; 
        if (abs(diff) > 90000)
            bsfi.pts = cts;
        else if ((dts - diff) > dts) {
            if (diffCount++ > 25) {
                // drop the frame
                diffCount = 0;
                continue;
            }
        }
        else if (diff > dts) {
            if (diffCount++ > 25) {
                // post a dummy frame
                diffCount = 0;
                dummyFrame.dataSize = computeBytes(dc, dc->periodSize);
                dummyFrame.pts = bsfi.pts;
                if (XPR_DPU_DeliverStreamBlock(dc->owner, &dummyFrame) < 0)
                    break;
                bsfi.pts += dts;
            }
        }

        if (bsfi.dataSize > 0) {
            if (XPR_DPU_DeliverStreamBlock(dc->owner, &bsfi) < 0)
                break;

            bsfi.pts += dts;
        }

        //printf("cts: %lld, pts: %lld, diff: %lld\n", cts, bsfi.pts, cts - bsfi.pts);
        //bsfi.pts += fixDTS(dc);
    }

    if (buffer)
        free(buffer);
    if (dummyBuffer)
        free(dummyBuffer);

    return dc->exitLoop ? 0 : -1;
}

static int waitForEncoderReady(DriverContext* dc)
{
    while (1) {
        snd_pcm_state_t state = snd_pcm_state(dc->pcmHandle);

        if (state == SND_PCM_STATE_OPEN ||
            state == SND_PCM_STATE_SETUP ||
            state == SND_PCM_STATE_PREPARED) {
            snd_pcm_start(dc->pcmHandle);
        } else if (state == SND_PCM_STATE_PAUSED) {
            snd_pcm_drop(dc->pcmHandle);
        } else if (state == SND_PCM_STATE_RUNNING)
            break;
    }

    return 0;
}

static void* workerThread(void* args)
{
    DriverContext* dc = (DriverContext*)args;

    while (!dc->exitLoop) {
        waitForEncoderReady(dc);
        loopReadBitstream(dc);
    }

    return (void*)0;
}

static void dump(DriverContext* dc)
{
    snd_pcm_dump(dc->pcmHandle, dc->log);
}

static void show_available_sample_formats(DriverContext* dc, snd_pcm_hw_params_t* params)
{
	snd_pcm_format_t format;

	fprintf(stderr, "Available formats:\n");
	for (format = 0; format < SND_PCM_FORMAT_LAST; format++) {
		if (snd_pcm_hw_params_test_format(dc->pcmHandle, params, format) == 0)
			fprintf(stderr, "- %s\n", snd_pcm_format_name(format));
	}
}

#define trace_line() fprintf(stderr, "%s:%d\n", __FILE__, __LINE__)

static int setParams(DriverContext* dc)
{
    int err;
    size_t n;
    unsigned int rate;
    snd_pcm_hw_params_t* params = 0;
    snd_pcm_sw_params_t* swparams = 0;

    snd_pcm_hw_params_alloca(&params);
    snd_pcm_sw_params_alloca(&swparams);
    err = snd_pcm_hw_params_any(dc->pcmHandle, params);

    if (err < 0) {
        fprintf(stderr, "Broken configuration for this PCM: no configurations available");
        return -1;
    }

    if (dc->verbose) {
        fprintf(stderr, "HW Params of device \"%s\":\n", snd_pcm_name(dc->pcmHandle));
        fprintf(stderr, "--------------------\n");
        snd_pcm_hw_params_dump(params, dc->log);
        fprintf(stderr, "--------------------\n");
    }

    if (dc->mmap) {
        snd_pcm_access_mask_t* mask = alloca(snd_pcm_access_mask_sizeof());
        snd_pcm_access_mask_none(mask);
        snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_INTERLEAVED);
        snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_NONINTERLEAVED);
        snd_pcm_access_mask_set(mask, SND_PCM_ACCESS_MMAP_COMPLEX);
        err = snd_pcm_hw_params_set_access_mask(dc->pcmHandle, params, mask);
    } else if (dc->interleaved)
        err = snd_pcm_hw_params_set_access(dc->pcmHandle, params,
                                           SND_PCM_ACCESS_RW_INTERLEAVED);
    else
        err = snd_pcm_hw_params_set_access(dc->pcmHandle, params,
                                           SND_PCM_ACCESS_RW_NONINTERLEAVED);

    if (err < 0) {
        fprintf(stderr, "Access type not available");
        return -1;
    }

    err = snd_pcm_hw_params_set_format(dc->pcmHandle, params, dc->format);

    if (err < 0) {
        fprintf(stderr, "Sample format non available");
        show_available_sample_formats(dc, params);
        return -1;
    }

    err = snd_pcm_hw_params_set_channels(dc->pcmHandle, params, dc->channels);

    if (err < 0) {
        fprintf(stderr, "Channels count non available");
        return -1;
    }

#if 0
    err = snd_pcm_hw_params_set_periods_min(dc->pcmHandle, params, 2);
    assert(err >= 0);
#endif

    rate = dc->sampleRate;
    err = snd_pcm_hw_params_set_rate_near(dc->pcmHandle, params, &dc->sampleRate, 0);
    assert(err >= 0);

    if ((float)rate * 1.05 < dc->sampleRate || (float)rate * 0.95 > dc->sampleRate) {
        if (dc->verbose) {
            char plugex[64];
            const char* pcmname = snd_pcm_name(dc->pcmHandle);
            fprintf(stderr, "Warning: rate is not accurate (requested = %iHz, got = %iHz)\n", rate, dc->sampleRate);

            if (!pcmname || strchr(snd_pcm_name(dc->pcmHandle), ':'))
                *plugex = 0;
            else
                snprintf(plugex, sizeof(plugex), "(-Dplug:%s)",
                         snd_pcm_name(dc->pcmHandle));

            fprintf(stderr, "         please, try the plug plugin %s\n",
                    plugex);
        }
    }

    rate = dc->sampleRate;

    if (dc->bufferTime == 0 && dc->bufferFrames == 0) {
        err = snd_pcm_hw_params_get_buffer_time_max(params, &dc->bufferTime, 0);
        assert(err >= 0);

        if (dc->bufferTime > 500000)
            dc->bufferTime = 500000;
    }

    if (dc->periodTime == 0 && dc->periodFrames == 0) {
        if (dc->bufferTime > 0)
            dc->periodTime = dc->bufferTime / 4;
        else
            dc->periodFrames = dc->bufferFrames / 4;
    }

    if (dc->periodTime > 0)
        err = snd_pcm_hw_params_set_period_time_near(dc->pcmHandle, params,
                &dc->periodTime, 0);
    else
        err = snd_pcm_hw_params_set_period_size_near(dc->pcmHandle, params,
                (snd_pcm_uframes_t*)&dc->periodFrames, 0);

    assert(err >= 0);

    if (dc->bufferTime > 0) {
        err = snd_pcm_hw_params_set_buffer_time_near(dc->pcmHandle, params,
                &dc->bufferTime, 0);
    } else {
        err = snd_pcm_hw_params_set_buffer_size_near(dc->pcmHandle, params,
                (snd_pcm_uframes_t*)&dc->bufferFrames);
    }

    assert(err >= 0);

#if 0
    dc->monotonic = snd_pcm_hw_params_is_monotonic(params);
#endif

    dc->canPause = snd_pcm_hw_params_can_pause(params);
    err = snd_pcm_hw_params(dc->pcmHandle, params);

    if (err < 0) {
        fprintf(stderr, "Unable to install hw params:");
        snd_pcm_hw_params_dump(params, dc->log);
        return -1;
    }

    snd_pcm_hw_params_get_period_size(params, (snd_pcm_uframes_t*)&dc->chunkSize, 0);
    snd_pcm_hw_params_get_buffer_size(params, (snd_pcm_uframes_t*)&dc->bufferSize);

    if (dc->chunkSize == dc->bufferSize) {
        fprintf(stderr, "Can't use period equal to buffer size (%u == %u)",
                dc->chunkSize, dc->bufferSize);
        return -1;
    }

    printf("dc->chunkSize: %d\n", dc->chunkSize);

    snd_pcm_sw_params_current(dc->pcmHandle, swparams);

    if (dc->availMin < 0)
        n = dc->chunkSize;
    else
        n = (double) rate * dc->availMin / 1000000;

    err = snd_pcm_sw_params_set_avail_min(dc->pcmHandle, swparams, n);
    /* round up to closest transfer boundary */
    n = dc->bufferSize;

#if 0
    if (dc->startDelay <= 0)
        dc->startThreshold = n + (double)rate * dc->startDelay / 1000000;
    else
        dc->startThreshold = (double)rate * dc->startDelay / 1000000;
#endif
    if (dc->startThreshold < 1)
        dc->startThreshold = 1;

#if 0
    if (dc->startThreshold > n)
        dc->startThreshold = n;
#endif

    err = snd_pcm_sw_params_set_start_threshold(dc->pcmHandle, swparams, dc->startThreshold);
    assert(err >= 0);

    if (dc->stopDelay <= 0)
        dc->stopThreshold = dc->bufferSize + (double) rate * dc->stopDelay / 1000000;
    else
        dc->stopThreshold = (double) rate * dc->stopDelay / 1000000;

    err = snd_pcm_sw_params_set_stop_threshold(dc->pcmHandle, swparams, dc->stopThreshold);
    assert(err >= 0);

    if (snd_pcm_sw_params(dc->pcmHandle, swparams) < 0) {
        fprintf(stderr, "unable to install sw params:");
        snd_pcm_sw_params_dump(swparams, dc->log);
        return -1;
    }

#if 0
    if (setup_chmap())
        return -1;
#endif

    if (dc->verbose)
        snd_pcm_dump(dc->pcmHandle, dc->log);

    dc->bitsPerSample = snd_pcm_format_physical_width(dc->format);
    dc->bitsPerFrame = dc->bitsPerSample * dc->channels;
    dc->chunkBytes = dc->chunkSize * dc->bitsPerFrame / 8;
    dc->audioBuffer = realloc(dc->audioBuffer, dc->chunkBytes);

    if (dc->audioBuffer == NULL) {
        fprintf(stderr, "not enough memory");
        return -1;
    }

    /* stereo VU-meter isn't always available... */
    if (dc->vumeter == VUMETER_STEREO) {
        if (dc->channels != 2 || !dc->interleaved || dc->verbose > 2)
            dc->vumeter = VUMETER_MONO;
    }

    /* show mmap buffer arragment */
    if (dc->mmap && dc->verbose) {
        const snd_pcm_channel_area_t* areas;
        snd_pcm_uframes_t offset, size = dc->chunkSize;
        int i;
        err = snd_pcm_mmap_begin(dc->pcmHandle, &areas, &offset, &size);

        if (err < 0) {
            fprintf(stderr, "snd_pcm_mmap_begin problem: %s", snd_strerror(err));
            return -1;
        }

        for (i = 0; i < dc->channels; i++)
            fprintf(stderr, "mmap_area[%i] = %p,%u,%u (%u)\n",
                    i, areas[i].addr, areas[i].first, areas[i].step,
                    snd_pcm_format_physical_width(dc->format));

        /* not required, but for sure */
        snd_pcm_mmap_commit(dc->pcmHandle, offset, 0);
    }

    dc->bufferFrames = dc->bufferSize;    /* for position test */

    return 0;
}

static int startCapture(DriverContext* dc)
{
    int err = 0;

    if (dc->pcmHandle)
        return 1;

    if (dc->codec == AV_FOURCC_PCMA) {
        dc->format = SND_PCM_FORMAT_A_LAW;
        dc->bitsPerSample = 8;
        dc->channels = 1;
        dc->sampleRate = 8000;
    }
    else if (dc->codec == AV_FOURCC_PCMU) {
        dc->format = SND_PCM_FORMAT_MU_LAW;
        dc->bitsPerSample = 8;
        dc->channels = 1;
        dc->sampleRate = 8000;
    }
    else if (dc->codec == AV_FOURCC_S16B) {
        dc->format = SND_PCM_FORMAT_S16_BE;
        dc->bitsPerSample = 16;
    }
    else if (dc->codec == AV_FOURCC_S16L) {
        dc->format = SND_PCM_FORMAT_S16_LE;
        dc->bitsPerSample = 16;
    }

    dc->periodSize = dc->sampleRate * dc->latency / 1000000;
    dc->bufferSize = dc->periodSize * 10;

    for (;;) {
        err = snd_pcm_open(&dc->pcmHandle, "default", SND_PCM_STREAM_CAPTURE, 0);

        if (err < 0) {
            fprintf(stderr, "failed to open pcm device, error: %s\n", snd_strerror(err));
            break;
        }
#if 0
        err = snd_pcm_set_params(dc->pcmHandle,
                                 dc->format,
                                 SND_PCM_ACCESS_RW_INTERLEAVED,
                                 dc->channels,
                                 dc->sampleRate,
                                 dc->softResample,
                                 dc->latency);

        if (err < 0) {
            fprintf(stderr, "failed to set pcm parameters, error: %s\n", snd_strerror(err));
            break;
        }
        snd_pcm_get_params(dc->pcmHandle, &dc->periodSize, &dc->bufferSize);
#else
#if 1
        snd_pcm_hw_params_t* hwparams = NULL;
        snd_pcm_hw_params_alloca(&hwparams);
        err = snd_pcm_hw_params_any(dc->pcmHandle, hwparams);
        if (dc->interleaved)
            err = snd_pcm_hw_params_set_access(dc->pcmHandle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED);
        else
            err = snd_pcm_hw_params_set_access(dc->pcmHandle, hwparams, SND_PCM_ACCESS_RW_NONINTERLEAVED);
        err = snd_pcm_hw_params_set_format(dc->pcmHandle, hwparams, dc->format);
        err = snd_pcm_hw_params_set_channels(dc->pcmHandle, hwparams, dc->channels);
        err = snd_pcm_hw_params_set_rate_near(dc->pcmHandle, hwparams, &dc->sampleRate, 0);
        err = snd_pcm_hw_params_set_period_size_near(dc->pcmHandle, hwparams, (snd_pcm_uframes_t*)&dc->periodSize, 0);
        err = snd_pcm_hw_params_set_buffer_size_near(dc->pcmHandle, hwparams, (snd_pcm_uframes_t*)&dc->bufferSize);
        err = snd_pcm_hw_params(dc->pcmHandle, hwparams);

        err = snd_pcm_hw_params_get_period_size(hwparams, (snd_pcm_uframes_t*)&dc->periodSize, 0);
        err = snd_pcm_hw_params_get_buffer_size(hwparams, (snd_pcm_uframes_t*)&dc->bufferSize);

        XPR_SYS_SetAudioClockFrequency(dc->sampleRate);
#else
        setParams(dc);
#endif
#endif
        //dump(dc);
        return 0;
    }

    return err;
}

static int stopCapture(DriverContext* dc)
{
    if (dc->pcmHandle) {
        int err = snd_pcm_drop(dc->pcmHandle);

        if (err < 0)
            fprintf(stderr, "failed to drop buffered pcm data, error: %s\n", snd_strerror(err));

        err = snd_pcm_close(dc->pcmHandle);

        if (err < 0)
            fprintf(stderr, "failed to close pcm device, error: %s\n", snd_strerror(err));

        if (!err)
            dc->pcmHandle = 0;
    }

    return 0;
}

static int startWorkerThread(DriverContext* dc)
{
    if (pthread_create(&dc->workerThread, NULL, workerThread, dc) < 0) {
        dc->workerThread = 0;
        return -1;
    }

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

    if (startCapture(dc) < 0)
        return -1;

    return startWorkerThread(dc);
}

static int stop(XPR_DPU* ctx)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;

    if (stopWorkerThread(dc) < 0)
        return -1;

    return stopCapture(dc);
}

static int getStreamCodec(XPR_DPU* ctx, int streamId)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;
    return dc->codec;
}

static int getStreamCount(XPR_DPU* ctx)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;
    (void)dc;
    return 1;
}

static int waitForReady(XPR_DPU* ctx)
{
    DriverContext* dc = (DriverContext*)ctx->privateData;
    return waitForEncoderReady(dc);
}

// Options
#define OFFSET(x) offsetof(DriverContext, x)
static XPR_DPU_Option options[] = {
    { "codec", "FourCC format codec, default [S16L]", OFFSET(codec), XPR_DPU_OPT_INT, { .i = AV_FOURCC_S16B }, 0, 0xFFFFFFFF, 0, 0 },
    { "format", NULL, OFFSET(format), XPR_DPU_OPT_INT, { .i = SND_PCM_FORMAT_S16_BE }, 0, 0xFFFFFFFF, 0, 0 },
    { "bitsPerSample", NULL, OFFSET(bitsPerSample), XPR_DPU_OPT_INT, { .i = 16 }, 8, 32, 0, 0 },
    { "sampleRate", NULL, OFFSET(sampleRate), XPR_DPU_OPT_INT, { .i = 44100 }, 8000, 48000, 0, 0 },
    { "channels", NULL, OFFSET(channels), XPR_DPU_OPT_INT, { .i = 2 }, 1, 2, 0, 0 },
    { "periodSize", NULL, OFFSET(periodSize), XPR_DPU_OPT_INT, { .i = 0 }, 1, 2, 0, 0 }, //FIXME
    { "periodTime", NULL, OFFSET(periodTime), XPR_DPU_OPT_INT, { .i = 0 }, 1, 2, 0, 0 }, //FIXME
    { "bufferSize", NULL, OFFSET(bufferSize), XPR_DPU_OPT_INT, { .i = 0 }, 1, 2, 0, 0 }, //FIXME
    { "bufferTime", NULL, OFFSET(bufferTime), XPR_DPU_OPT_INT, { .i = 0 }, 1, 2, 0, 0 }, //FIXME
    { "availMin", NULL, OFFSET(availMin), XPR_DPU_OPT_INT, { .i = -1 }, INT_MIN, INT_MAX, 0, 0 },
    { "interleaved", NULL, OFFSET(interleaved), XPR_DPU_OPT_INT, { .i = 1 }, 0, 1, 0, 0 },
    // 0 = disallow alsa-lib resample stream, 1 = allow resampling
    { "softResample", "0 = disallow alsa-lib resample stream, 1 = allow resampling", OFFSET(softResample), XPR_DPU_OPT_INT, { .i = 0 }, 0, 1, 0, 0 },
    // overall latency in us
    { "latency", "overall latency in us, default 20000", OFFSET(latency), XPR_DPU_OPT_INT, { .i = 20000 }, 0, 1000000, 0, 0 },
    { "verbose", NULL, OFFSET(verbose), XPR_DPU_OPT_INT, { .i = 0 }, 0, 9, 0, 0 },
    { "volume", NULL, OFFSET(volume), XPR_DPU_OPT_INT, { .i = 80 }, 0, 100, 0, 0 },
    { NULL },
};

// Registry
///////////////////////////////////////////////////////////////////////////////
struct XPR_DPU_Driver xpr_dpu_driver_alsapcm = {
    .name = "alsapcm",
    .description = "Alsa PCM DPU driver",
    .identifier = XPR_DPU_ID_ALSAPCM,
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

