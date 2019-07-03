/*
 * File: xpr.h
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * 跨平台运行库
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * Project       : xpr
 * Author        : Varphone Wong <varphone@qq.com>
 * File Created  : 2016-12-13 10:39:16 Tuesday, 13 December
 * Last Modified : 2019-07-03 05:42:54 Wednesday, 3 July
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
#ifndef XPR_XPR_H
#define XPR_XPR_H

#ifdef HAVE_CONFIG_H
#  if defined(WIN32) || defined(_WIN32)
#    include "xpr_config_win32.h"
#  else
#    include "xpr_config.h"
#  endif /* defined(WIN32) || defined(_WIN32) */
#endif /* HAVE_CONFIG_H */

#include "xpr_common.h"
#include "xpr_errno.h"

#ifdef HAVE_XPR_ADC
#include "xpr_adc.h"
#endif /* HAVE_XPR_ADC */

#ifdef HAVE_XPR_ARP
#include "xpr_arp.h"
#endif /* HAVE_XPR_ARP */

#ifdef HAVE_XPR_ARR
#include "xpr_arr.h"
#endif /* HAVE_XPR_ARR */

#ifdef HAVE_XPR_ATOMIC
#include "xpr_atomic.h"
#endif /* HAVE_XPR_ATOMIC */

#ifdef HAVE_XPR_AVFRAME
#include "xpr_avframe.h"
#endif /* HAVE_XPR_AVFAME */

#ifdef HAVE_XPR_BASE64
#include "xpr_base64.h"
#endif /* HAVE_XPR_BASE64 */

#ifdef HAVE_XPR_BITVECTOR
#include "xpr_bitvector.h"
#endif /* HAVE_XPR_BITVECTOR */

#ifdef HAVE_XPR_CRC
#include "xpr_crc.h"
#endif /* HAVE_XPR_CRC */

#ifdef HAVE_XPR_DEVCAPS
#include "xpr_devcaps.h"
#endif /* HAVE_XPR_DEVCAPS */

#ifdef HAVE_XPR_DPU
#include "xpr_dpu.h"
#endif /* HAVE_XPR_DPU */

#ifdef HAVE_XPR_DRM
#include "xpr_drm.h"
#endif /* HAVE_XPR_DRM */

#ifdef HAVE_XPR_FIFO
#include "xpr_fifo.h"
#endif /* HAVE_XPR_FIFO */

#ifdef HAVE_XPR_FILE
#include "xpr_file.h"
#endif /* HAVE_XPR_FILE */

#ifdef HAVE_XPR_FQ
#include "xpr_fq.h"
#endif /* HAVE_XPR_FQ */

#ifdef HAVE_XPR_GPIO
#include "xpr_GPIO.h"
#endif /* HAVE_XPR_GPIO */

#ifdef HAVE_XPR_H264
#include "xpr_h264.h"
#endif /* HAVE_XPR_H264 */

#ifdef HAVE_XPR_ICMP
#include "xpr_icmp.h"
#endif /* HAVE_XPR_ICMP */

#ifdef HAVE_XPR_JSON
#include "xpr_json.h"
#endif /* HAVE_XPR_JSON */

#ifdef HAVE_XPR_LIST
#include "xpr_list.h"
#endif /* HAVE_XPR_LIST */

#ifdef HAVE_XPR_MCDEC
#include "xpr_mcdec.h"
#endif /* HAVE_XPR_MCDEC */

#ifdef HAVE_XPR_MCVR
#include "xpr_mcvr.h"
#endif /* HAVE_XPR_MCVR */

#ifdef HAVE_XPR_MD5
#include "xpr_md5.h"
#endif /* HAVE_XPR_MD5 */

#ifdef HAVE_XPR_MEM
#include "xpr_mem.h"
#endif /* HAVE_XPR_MEM */

#ifdef HAVE_XPR_META
#include "xpr_meta.h"
#endif /* HAVE_XPR_META */

#ifdef HAVE_XPR_ONVIF
#include "xpr_onvif.h"
#endif /* HAVE_XPR_ONVIF */

#ifdef HAVE_XPR_OSD
#include "xpr_osd.h"
#endif /* HAVE_XPR_OSD */

#ifdef HAVE_XPR_PES
#include "xpr_pes.h"
#endif /* HAVE_XPR_PES */

#ifdef HAVE_XPR_PLUGIN
#include "xpr_plugin.h"
#endif /* HAVE_XPR_PLUGIN */

#ifdef HAVE_XPR_RTSP
#include "xpr_rtsp.h"
#endif /* HAVE_XPR_RTSP */

#ifdef HAVE_XPR_RTSP_IDD
#include "xpr_rtsp_idd.h"
#endif /* HAVE_XPR_RTSP_IDD */

#ifdef HAVE_XPR_SHA1
#include "xpr_sha1.h"
#endif /* HAVE_XPR_SHA1 */

#ifdef HAVE_XPR_STREAMBLOCK
#include "xpr_streamblock.h"
#endif /* HAVE_XPR_STREAMBLOCK */

#ifdef HAVE_XPR_SYNC
#include "xpr_sync.h"
#endif /* HAVE_XPR_SYNC */

#ifdef HAVE_XPR_SYS
#include "xpr_sys.h"
#endif /* HAVE_XPR_SYS */

#ifdef HAVE_XPR_THREAD
#include "xpr_thread.h"
#endif /* HAVE_XPR_THREAD */

#ifdef HAVE_XPR_TIMER
#include "xpr_timer.h"
#endif /* HAVE_XPR_TIMER */

#ifdef HAVE_XPR_UIO
#include "xpr_uio.h"
#endif /* HAVE_XPR_UIO */

#ifdef HAVE_XPR_UPS
#include "xpr_ups.h"
#endif /* HAVE_XPR_UPS */

#ifdef HAVE_XPR_URL
#include "xpr_url.h"
#endif /* HAVE_XPR_URL */

#ifdef HAVE_XPR_UTILS
#include "xpr_utils.h"
#endif /* HAVE_XPR_UTILS */

#ifdef HAVE_XPR_XML
#include "xpr_xml.h"
#endif /* HAVE_XPR_XML */

#endif /* XPR_XPR_H */
