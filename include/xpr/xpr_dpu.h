/*
 * File: xpr_dpu.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 数据处理单元
 * 
 * 处理或提供实时或非实时性各类数据, 如获取或播放音视频 ES 流,
 * 获取系统各类报警事件等等
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2014-11-21 12:50:43 Friday, 21 November
 * Last Modified : 2019-07-03 05:21:28 Wednesday, 3 July
 * Modified By   : Varphone Wong <varphone@qq.com>
 * ---------------------------------------------------------------------------
 * Copyright (C) 2012 - 2019 CETC55, Technology Development CO.,LTD.
 * Copyright (C) 2012 - 2019 Varphone Wong, Varphone.com.
 * All rights reserved.
 * ---------------------------------------------------------------------------
 * HISTORY:
 * 2019-07-03   varphone    更新版权信息
 * 2014-11-21   varphone    初始版本建立
 */
#ifndef XPR_DPU_H
#define XPR_DPU_H

#include <stdint.h>
#include <xpr/xpr_avframe.h>
#include <xpr/xpr_common.h>
#include <xpr/xpr_streamblock.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XPR_DPU_CTS_UNITS      1000000
#define XPR_DPU_MAX_CALLBACKS  16

// Forwards
struct XPR_DPU;

/// @brief Type of XPR_DPU
typedef struct XPR_DPU XPR_DPU;

/// @brief Type of XPR_DPU events
enum XPR_DPU_EventType {
    XPR_DPU_EV_STARTED,  ///< DPU Started
    XPR_DPU_EV_STOPPED,  ///< DPU Stopped
    XPR_DPU_EV_RUNNING,  ///< DPU running
    XPR_DPU_EV_FMTCHGD,  ///< DPU data format changed
};

/// @brief Initialize the library
/// @retval 0   success
/// @retval -1  failure
XPR_API int XPR_DPU_Init(void);

/// @brief Finalize the library
/// @return no value returns
XPR_API void XPR_DPU_Fini(void);

/// @brief Open a new data process unit
/// @param [in] args        JSON string formated arguments
/// @return a pointer of the DPU instance, NULL on failure
XPR_API XPR_DPU* XPR_DPU_Open(const char* args);

/// @brief Close a opened data process unit
/// @param [in] dpu         DPU instance
/// @retval 0   success
/// @retval -1  failure
XPR_API int XPR_DPU_Close(XPR_DPU* dpu);

/// @brief Start data process unit
/// @param [in] dpu         DPU instance
/// @retval 0   success
/// @retval -1  failure
XPR_API int XPR_DPU_Start(XPR_DPU* dpu);

/// @brief Stop data process unit
/// @param [in] dpu         DPU instance
/// @retval 0   success
/// @retval -1  failure
XPR_API int XPR_DPU_Stop(XPR_DPU* dpu);

/// @brief Audio/Video frame callback
/// @param [in] dpu         DPU instance
/// @param [in] frame       Audio/Video frame
/// @param [in] opaque      User context data
/// @retval 0   continue
/// @retval -1  break
typedef int (*XPR_DPU_AVFrameCallback)(XPR_DPU* dpu, const XPR_AVFrame* frame,
                                       void* opaque);

/// @brief Stream block callback
/// @param [in] dpu         DPU instance
/// @param [in] block       Stream block data
/// @param [in] opaque      User context data
/// @retval 0   continue
/// @retval -1  break
typedef int (*XPR_DPU_StreamBlockCallback)(XPR_DPU* dpu,
                                           const XPR_StreamBlock* block,
                                           void* opaque);

/// @brief Add an audio/video frame callback
/// @param [in] dpu         DPU instance
/// @param [in] cb          Callback routine
/// @param [in] opaque      User context data
/// @retval 0   success
/// @retval -1  failure
XPR_API int XPR_DPU_AddAVFrameCallback(XPR_DPU* dpu, XPR_DPU_AVFrameCallback cb,
                                       void* opaque);

/// @brief Add a stream block callback
/// @param [in] dpu         DPU instance
/// @param [in] cb          Callback routine
/// @param [in] opaque      User context data
/// @retval 0   success
/// @retval -1  failure
XPR_API int XPR_DPU_AddStreamBlockCallback(XPR_DPU* dpu,
                                           XPR_DPU_StreamBlockCallback cb,
                                           void* opaque);

/// @brief Delete an audio/video frame callback
/// @param [in] dpu         DPU instance
/// @param [in] cb          Callback routine
/// @param [in] opaque      User context data
/// @retval 0   success
/// @retval -1  failure
XPR_API int XPR_DPU_DeleteAVFrameCallback(XPR_DPU* dpu,
                                          XPR_DPU_AVFrameCallback cb,
                                          void* opaque);

/// @brief Delete a stream block callback
/// @param [in] dpu         DPU instance
/// @param [in] cb          Callback routine
/// @param [in] opaque      User context data
/// @retval 0   success
/// @retval -1  failure
XPR_API int XPR_DPU_DeleteStreamBlockCallback(XPR_DPU* dpu,
                                              XPR_DPU_StreamBlockCallback cb,
                                              void* opaque);

/// @brief Data process unit event callback
/// @param [in] dpu         DPU instance
/// @param [in] cb          Callback routine
/// @param [in] opaque      User context data
/// @retval 0   success
/// @retval -1  failure
typedef int (*XPR_DPU_EventCallback)(XPR_DPU* dpu, int event,
                                     const void* eventData, int eventDataSize,
                                     void* opaque);

/// @brief Add an event callback
/// @param [in] dpu         DPU instance
/// @param [in] cb          Callback routine
/// @param [in] opaque      User context data
/// @retval 0   success
/// @retval -1  failure
XPR_API int XPR_DPU_AddEventCallback(XPR_DPU* dpu, XPR_DPU_EventCallback cb,
                                     void* opaque);

/// @brief Delete an event callback
/// @param [in] dpu         DPU instance
/// @param [in] cb          Callback routine
/// @param [in] opaque      User context data
/// @retval 0   success
/// @retval -1  failure
XPR_API int XPR_DPU_DeleteEventCallback(XPR_DPU* dpu, XPR_DPU_EventCallback cb,
                                        void* opaque);

/// @brief Reference clock callback
/// @param [in] opaque      User context data
/// @return us precision timestamp
typedef int64_t (*XPR_DPU_RefClockCallback)(void* opaque);

/// @brief Set reference clock callback
/// @param [in] dpu         DPU instance
/// @param [in] cb          Callback routine
/// @param [in] opaque      User context data
/// @retval 0   success
/// @retval -1  failure
XPR_API int XPR_DPU_SetRefClockCallback(XPR_DPU* dpu,
                                        XPR_DPU_RefClockCallback cb,
                                        void* opaque);

/// @brief Get reference clock callback
/// @param [in] dpu         DPU instance
/// @param [in] cb          To obtain callback routine pointer
/// @param [in] opaque      To obtain User context data
/// @retval 0   success
/// @retval -1  failure
XPR_API int XPR_DPU_GetRefClockCallback(XPR_DPU* dpu,
                                        XPR_DPU_RefClockCallback* cb,
                                        void** opaque);

/// @brief Set current timestamp
/// @param [in] dpu         DPU instance
/// @param [in] ts          Timestamp
/// @param [in] units       Timestamp precision
/// @retval 0   success
/// @retval -1  failure
XPR_API int XPR_DPU_SetCTS(XPR_DPU* dpu, int64_t ts, int units);

/// @brief Get current timestamp
/// @param [in] dpu         DPU instance
/// @param [in] units       Timestamp precision
/// @return timestamp
XPR_API int64_t XPR_DPU_GetCTS(XPR_DPU* dpu, int units);

/// @brief Get system timestamp
/// @param [in] units       Timestamp precision
/// @return timestamp
XPR_API int64_t XPR_DPU_GetSystemCTS(int units);

/// @brief Set option value
/// @param [in] dpu         DPU instance
/// @param [in] name        Option's name
/// @param [in] data        Option's value data
/// @param [in] length      Option's value data length
XPR_API int XPR_DPU_SetOption(XPR_DPU* dpu, const char* name, const void* data,
                              int length);

/// @brief Get option value
/// @param [in] dpu         DPU instance
/// @param [in] name        Option's name
/// @param [in] buffer      Buffer to obtain the option value data
/// @param [in] size        Size of the Buffer to obtain the option value data
/// @retval 0   success
/// @retval -1  failure
XPR_API int XPR_DPU_GetOption(XPR_DPU* dpu, const char* name, void* buffer,
                              int* size);

/// @brief Print all options to stdout
/// @param [in] dpu         DPU instance
/// @return no value returns
XPR_API void XPR_DPU_ShowOptions(XPR_DPU* dpu);

/// @brief Get stream codec
/// @param [in] dpu         DPU instance
/// @param [in] streamId    Stream index, based on 0
/// @return fourcc codec
XPR_API int XPR_DPU_GetStreamCodec(XPR_DPU* dpu, int streamId);

/// @brief Get stream count
/// @param [in] dpu         DPU instance
/// @return number of streams supported
XPR_API int XPR_DPU_GetStreamCount(XPR_DPU* dpu);

XPR_API int XPR_DPU_GetStreamParam(XPR_DPU* dpu, int streamId,
                                   const char* param, void* buffer, int* size);

XPR_API int XPR_DPU_SetStreamParam(XPR_DPU* dpu, int streamId,
                                   const char* param, const void* data,
                                   int length);

/// @brief Wait for the DPU ready
/// @param [in] dpu         DPU instance
/// @retval 0   success
/// @retval -1  failure
XPR_API int XPR_DPU_WaitForReady(XPR_DPU* dpu);

XPR_API int XPR_DPU_DeliverAVFrame(XPR_DPU* ctx, const XPR_AVFrame* frame);

XPR_API int XPR_DPU_DeliverEvent(XPR_DPU* ctx, int event, const void* eventData,
                                 int eventDataSize);

XPR_API int XPR_DPU_DeliverStreamBlock(XPR_DPU* ctx,
                                       const XPR_StreamBlock* block);

#ifdef __cplusplus
}
#endif

#endif // XPR_DPU_H
