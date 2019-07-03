#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_TIME_H
#include <time.h>
#endif
#include "xpr_dpu_options.h"
#include <xpr/xpr_avframe.h>
#include <xpr/xpr_dpu.h>
#include <xpr/xpr_dpupriv.h>

static XPR_DPU_Driver* gFirstDPUDriver = NULL;

XPR_API void XPR_DPU_Register(XPR_DPU_Driver* drv)
{
    XPR_DPU_Driver** p;
    p = &gFirstDPUDriver;

    while (*p != NULL)
        p = &(*p)->next;

    *p = drv;
    drv->next = NULL;
}

XPR_API void XPR_DPU_RegisterAll(void)
{
    static int initialized = 0;

    if (initialized)
        return;

    initialized = 1;

#if HAVE_XPR_DPU_DRIVER_ALSAPCM
    extern XPR_DPU_Driver xpr_dpu_driver_alsapcm;
    XPR_DPU_Register(&xpr_dpu_driver_alsapcm);
#endif
#if HAVE_XPR_DPU_DRIVER_A5SVIDEO
    extern XPR_DPU_Driver xpr_dpu_driver_a5svideo;
    XPR_DPU_Register(&xpr_dpu_driver_a5svideo);
#endif
#if HAVE_XPR_DPU_DRIVER_A5SYUV
    extern XPR_DPU_Driver xpr_dpu_driver_a5syuv;
    XPR_DPU_Register(&xpr_dpu_driver_a5syuv);
#endif
#if HAVE_XPR_DPU_DRIVER_G711TEST
    extern XPR_DPU_Driver xpr_dpu_driver_g711test;
    XPR_DPU_Register(&xpr_dpu_driver_g711test);
#endif
#if HAVE_XPR_DPU_DRIVER_PCMTEST
    extern XPR_DPU_Driver xpr_dpu_driver_pcmtest;
    XPR_DPU_Register(&xpr_dpu_driver_pcmtest);
#endif
#if HAVE_XPR_DPU_DRIVER_MDSD
    extern XPR_DPU_Driver xpr_dpu_driver_mdsd;
    XPR_DPU_Register(&xpr_dpu_driver_mdsd);
#endif
}

XPR_API XPR_DPU_Driver* XPR_DPU_FindDriver(enum XPR_DPU_Id id)
{
    XPR_DPU_Driver* p = NULL;
    p = gFirstDPUDriver;
    while (p) {
        if (p->identifier == id)
            return p;

        p = p->next;
    }
    return p;
}

static const char* gDriverNames[] = {
    [XPR_DPU_ID_INVALID] = "null",      //
    [XPR_DPU_ID_ALSAPCM] = "alsapcm",   //
    [XPR_DPU_ID_A2AAC] = "a2aac",       //
    [XPR_DPU_ID_A2G711A] = "a2g711a",   //
    [XPR_DPU_ID_A2G711U] = "a2g711u",   //
    [XPR_DPU_ID_A2G726] = "a2g726",     //
    [XPR_DPU_ID_A2VIDEO] = "a2video",   //
    [XPR_DPU_ID_A5SG711A] = "a5sg711u", //
    [XPR_DPU_ID_A5SG711U] = "a5sg711u", //
    [XPR_DPU_ID_A5SG726] = "a5sg726",   //
    [XPR_DPU_ID_A5SVIDEO] = "a5svideo", //
    [XPR_DPU_ID_A5SYUV] = "a5syuv",     //
    [XPR_DPU_ID_G711TEST] = "g711test", //
    [XPR_DPU_ID_PCMTEST] = "pcmtest",   //
    [XPR_DPU_ID_MDSD] = "mdsd",         //
    [XPR_DPU_ID_MAX] = 0,               //
};

XPR_API enum XPR_DPU_Id XPR_DPU_FindDriverId(const char* name)
{
    int i = 0;

    if (name) {
        for (i = XPR_DPU_ID_INVALID + 1; i < XPR_DPU_ID_MAX; i++) {
            if (!gDriverNames[i])
                continue;
            if (strcmp(gDriverNames[i], name) == 0)
                return i;
        }
    }
    return XPR_DPU_ID_INVALID;
}

XPR_API XPR_DPU* XPR_DPU_AllocContext(void)
{
    return (XPR_DPU*)calloc(sizeof(XPR_DPU), 1);
}

XPR_API void XPR_DPU_FreeContext(XPR_DPU* ctx)
{
    free(ctx);
}

XPR_API int XPR_DPU_OpenContext(XPR_DPU* ctx, const XPR_DPU_Driver* drv)
{
    if (!ctx || !drv)
        return -1;

    if (drv->privateDataSize > 0) {
        if (!ctx->privateData) {
            ctx->privateData = calloc(1, drv->privateDataSize);

            if (!ctx->privateData)
                return -1;
        }
    }
    else {
        ctx->privateData = NULL;
    }

    ctx->driver = drv;

    if (drv && drv->options)
        XPR_DPU_SetDefaultOptions(ctx);

    if (ctx->driver->init)
        return ctx->driver->init(ctx);

    return 0;
}

XPR_API int XPR_DPU_CloseContext(XPR_DPU* ctx)
{
    if (!ctx)
        return -1;

    if (ctx->driver && ctx->driver->dein)
        ctx->driver->dein(ctx);

    if (ctx->privateData) {
        free(ctx->privateData);
        ctx->privateData = NULL;
    }

    return 0;
}

XPR_API const char* XPR_DPU_GetDriverName(const XPR_DPU* ctx)
{
    if (!ctx)
        return NULL;

    if (!ctx->driver)
        return NULL;

    return ctx->driver->name;
}

XPR_API int XPR_DPU_GetDriverIdentifier(const XPR_DPU* ctx)
{
    if (!ctx)
        return 0;

    if (!ctx->driver)
        return 0;

    return ctx->driver->identifier;
}

XPR_API int XPR_DPU_GetDriverCapabilities(const XPR_DPU* ctx)
{
    if (!ctx)
        return XPR_DPU_CAP_HAVE_NOTHING;

    if (!ctx->driver)
        return XPR_DPU_CAP_HAVE_NOTHING;

    return ctx->driver->capabilities;
}

XPR_API int XPR_DPU_Init(void)
{
    XPR_DPU_RegisterAll();
    return 0;
}

XPR_API void XPR_DPU_Fini(void)
{
}

XPR_API static const char* getDriverName(const char* args)
{
    if (strstr(args, "\"alsapcm\""))
        return "alsapcm";
    if (strstr(args, "\"a5svideo\""))
        return "a5svideo";
    if (strstr(args, "\"a5syuv\""))
        return "a5syuv";
    if (strstr(args, "\"g711test\""))
        return "g711test";
    if (strstr(args, "\"pcmtest\""))
        return "pcmtest";
    if (strstr(args, "\"mdsd\""))
        return "mdsd";
    return NULL;
}

XPR_API XPR_DPU* XPR_DPU_Open(const char* args)
{
    enum XPR_DPU_Id id = 0;
    XPR_DPU* ctx = 0;
    XPR_DPU_Driver* drv = 0;
    const char* drvName = getDriverName(args);
    if (!drvName) {
        fprintf(stderr, "Unable to find driver for: %s\n", args);
        return 0;
    }
    id = XPR_DPU_FindDriverId(drvName);
    if (id == XPR_DPU_ID_INVALID) {
        fprintf(stderr, "Unregistered driver: %s\n", drvName);
        return 0;
    }

    drv = XPR_DPU_FindDriver(id);
    if (drv) {
        ctx = XPR_DPU_AllocContext();
        if (ctx) {
            if (XPR_DPU_OpenContext(ctx, drv) == 0)
                return (XPR_DPU*)ctx;
            XPR_DPU_FreeContext(ctx);
        }
    }
    return 0;
}

XPR_API int XPR_DPU_Close(XPR_DPU* ctx)
{
    if (XPR_DPU_CloseContext(ctx) < 0)
        return -1;
    XPR_DPU_FreeContext(ctx);
    return 0;
}

XPR_API int XPR_DPU_Start(XPR_DPU* ctx)
{
    if (!ctx)
        return -1;

    if (!ctx->driver)
        return -1;

    if (!ctx->driver->start)
        return -1;

    return ctx->driver->start(ctx);
}

XPR_API int XPR_DPU_Stop(XPR_DPU* ctx)
{
    if (!ctx)
        return -1;

    if (!ctx->driver)
        return -1;

    if (!ctx->driver->stop)
        return -1;

    return ctx->driver->stop(ctx);
}

XPR_API int XPR_DPU_AddAVFrameCallback(XPR_DPU* ctx, XPR_DPU_AVFrameCallback cb,
                                       void* opaque)
{
    int i = 0;

    if (!ctx)
        return -1;

    for (; i < XPR_DPU_MAX_CALLBACKS; i++) {
        if (ctx->avFrameCallbacks[i].cb)
            continue;
        ctx->avFrameCallbacks[i].cb = cb;
        ctx->avFrameCallbacks[i].opaque = opaque;
        return 0;
    }

    return -1;
}

XPR_API int XPR_DPU_DeleteAVFrameCallback(XPR_DPU* ctx,
                                          XPR_DPU_AVFrameCallback cb,
                                          void* opaque)
{
    int i = 0;

    if (!ctx)
        return -1;

    for (; i < XPR_DPU_MAX_CALLBACKS; i++) {
        if (ctx->avFrameCallbacks[i].cb == cb &&
            ctx->avFrameCallbacks[i].opaque == opaque) {
            ctx->avFrameCallbacks[i].cb = NULL;
            ctx->avFrameCallbacks[i].opaque = NULL;
            return 0;
        }
    }

    return -1;
}

XPR_API int XPR_DPU_AddStreamBlockCallback(XPR_DPU* ctx,
                                           XPR_DPU_StreamBlockCallback cb,
                                           void* opaque)
{
    int i = 0;

    if (!ctx)
        return -1;

    for (; i < XPR_DPU_MAX_CALLBACKS; i++) {
        if (ctx->streamBlockCallbacks[i].cb)
            continue;
        ctx->streamBlockCallbacks[i].cb = cb;
        ctx->streamBlockCallbacks[i].opaque = opaque;
        return 0;
    }

    return -1;
}

XPR_API int XPR_DPU_DeleteStreamBlockCallback(XPR_DPU* ctx,
                                              XPR_DPU_StreamBlockCallback cb,
                                              void* opaque)
{
    int i = 0;

    if (!ctx)
        return -1;

    for (; i < XPR_DPU_MAX_CALLBACKS; i++) {
        if (ctx->streamBlockCallbacks[i].cb == cb &&
            ctx->streamBlockCallbacks[i].opaque == opaque) {
            ctx->streamBlockCallbacks[i].cb = NULL;
            ctx->streamBlockCallbacks[i].opaque = NULL;
            return 0;
        }
    }

    return -1;
}

XPR_API int XPR_DPU_AddEventCallback(XPR_DPU* ctx, XPR_DPU_EventCallback cb,
                                     void* opaque)
{
    int i = 0;

    if (!ctx)
        return -1;

    for (; i < XPR_DPU_MAX_CALLBACKS; i++) {
        if (ctx->eventCallbacks[i].cb)
            continue;
        ctx->eventCallbacks[i].cb = cb;
        ctx->eventCallbacks[i].opaque = opaque;
        return 0;
    }

    return -1;
}

XPR_API int XPR_DPU_DeleteEventCallback(XPR_DPU* ctx, XPR_DPU_EventCallback cb,
                                        void* opaque)
{
    int i = 0;

    if (!ctx)
        return -1;

    for (; i < XPR_DPU_MAX_CALLBACKS; i++) {
        if (ctx->eventCallbacks[i].cb == cb &&
            ctx->eventCallbacks[i].opaque == opaque) {
            ctx->eventCallbacks[i].cb = NULL;
            ctx->eventCallbacks[i].opaque = NULL;
            return 0;
        }
    }

    return -1;
}

XPR_API int XPR_DPU_SetRefClockCallback(XPR_DPU* ctx,
                                        XPR_DPU_RefClockCallback cb,
                                        void* opaque)
{
    if (!ctx)
        return -1;
    ctx->refClockCallback = cb;
    ctx->refClockCallbackOpaque = opaque;
    return 0;
}

XPR_API int XPR_DPU_GetRefClockCallback(XPR_DPU* ctx,
                                        XPR_DPU_RefClockCallback* cb,
                                        void** opaque)
{
    if (!ctx || !cb || !opaque)
        return -1;

    *cb = ctx->refClockCallback;
    *opaque = ctx->refClockCallbackOpaque;

    return 0;
}

XPR_API int XPR_DPU_SetCTS(XPR_DPU* ctx, int64_t ts, int units)
{
    if (!ctx)
        return -1;
    if (units == 0)
        ctx->cts = ts;
    else
        ctx->cts = ts * XPR_DPU_CTS_UNITS / units;
    return 0;
}

XPR_API int64_t XPR_DPU_GetCTS(XPR_DPU* ctx, int units)
{
    int64_t cts = 0;

    if (!ctx)
        return 0;
    if (ctx->refClockCallback)
        cts = ctx->refClockCallback(ctx->refClockCallbackOpaque);
    else
        cts = ctx->cts;
    if (cts == 0)
        cts = XPR_DPU_GetSystemCTS(units);
    else {
        if (units > 0)
            return cts * units / XPR_DPU_CTS_UNITS;
    }
    return cts;
}

static int64_t systemCTS(void)
{
#ifdef HAVE_TIME_H
    struct timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);
    return (int64_t)tp.tv_sec * 1000000 + (int64_t)tp.tv_nsec / 1000;
#else
    return 0;
#endif
}

XPR_API int64_t XPR_DPU_GetSystemCTS(int units)
{
    if (units == 0)
        return systemCTS();
    return systemCTS() * units / XPR_DPU_CTS_UNITS;
}

XPR_API int XPR_DPU_GetStreamCodec(XPR_DPU* ctx, int streamId)
{
    if (!ctx)
        return AV_FOURCC_NULL;

    if (!ctx->driver)
        return AV_FOURCC_NULL;

    if (!ctx->driver->getStreamCodec)
        return AV_FOURCC_NULL;

    return ctx->driver->getStreamCodec(ctx, streamId);
}

XPR_API int XPR_DPU_GetStreamCount(XPR_DPU* ctx)
{
    if (!ctx)
        return -1;

    if (!ctx->driver)
        return -1;

    if (!ctx->driver->getStreamCount)
        return -1;

    return ctx->driver->getStreamCount(ctx);
}

XPR_API int XPR_DPU_GetStreamParam(XPR_DPU* ctx, int streamId,
                                   const char* param, void* buffer, int* size)
{
    if (!ctx)
        return -1;

    if (!ctx->driver)
        return -1;

    if (!ctx->driver->getStreamParam)
        return -1;

    return ctx->driver->getStreamParam(ctx, streamId, param, buffer, size);
}

XPR_API int XPR_DPU_SetStreamParam(XPR_DPU* ctx, int streamId,
                                   const char* param, const void* data,
                                   int length)
{
    if (!ctx)
        return -1;

    if (!ctx->driver)
        return -1;

    if (!ctx->driver->setStreamParam)
        return -1;

    return ctx->driver->setStreamParam(ctx, streamId, param, data, length);
}

XPR_API int XPR_DPU_WaitForReady(XPR_DPU* ctx)
{
    if (!ctx)
        return -1;

    if (!ctx->driver)
        return -1;

    if (!ctx->driver->waitForReady)
        return -1;

    return ctx->driver->waitForReady(ctx);
}

XPR_API int XPR_DPU_DeliverAVFrame(XPR_DPU* ctx, const XPR_AVFrame* frame)
{
    int i = 0;
    int error = 0;
    for (; i < XPR_DPU_MAX_CALLBACKS; i++) {
        if (ctx->avFrameCallbacks[i].cb) {
            error = ctx->avFrameCallbacks[i].cb(
                (XPR_DPU*)ctx, frame, ctx->avFrameCallbacks[i].opaque);
            if (error < 0)
                break;
        }
    }
    return error;
}

XPR_API int XPR_DPU_DeliverEvent(XPR_DPU* ctx, int event, const void* eventData,
                                 int eventDataSize)
{
    int i = 0;
    int error = 0;
    for (; i < XPR_DPU_MAX_CALLBACKS; i++) {
        if (ctx->eventCallbacks[i].cb) {
            error = ctx->eventCallbacks[i].cb((XPR_DPU*)ctx, event, eventData,
                                              eventDataSize,
                                              ctx->eventCallbacks[i].opaque);
            if (error < 0)
                break;
        }
    }
    return error;
}

XPR_API int XPR_DPU_DeliverStreamBlock(XPR_DPU* ctx,
                                       const XPR_StreamBlock* block)
{
    int i = 0;
    int error = 0;
    for (; i < XPR_DPU_MAX_CALLBACKS; i++) {
        if (ctx->streamBlockCallbacks[i].cb) {
            error = ctx->streamBlockCallbacks[i].cb(
                (XPR_DPU*)ctx, block, ctx->streamBlockCallbacks[i].opaque);
            if (error < 0)
                break;
        }
    }
    return error;
}
