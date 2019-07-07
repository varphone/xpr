# src/CMakeLists.txt
# @author Varphone Wong <varphone@qq.com>
# @date 2018/11/16

project(xpr)

if(HAVE_XPR_ADC)
    list(APPEND SOURCES xpr_adc.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_adc.h)
endif()

if(HAVE_XPR_ARP)
    if(UNIX)
        list(APPEND SOURCES xpr_arp_unix.c)
    endif()
    if(MSVC)
        list(APPEND SOURCES xpr_arp_win32.c)
    endif()
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_arp.h)
endif()

if(HAVE_XPR_ARR)
    list(APPEND SOURCES xpr_arr.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_arr.h)
endif()

if(HAVE_XPR_AVFRAME)
    list(APPEND SOURCES xpr_avframe.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_avframe.h)
endif()

if(HAVE_XPR_BASE64)
    list(APPEND SOURCES xpr_base64.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_base64.h)
endif()

if(HAVE_XPR_BITVECTOR)
    list(APPEND SOURCES xpr_bitvector.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_bitvector.h)
endif()

if(HAVE_XPR_CRC)
    list(APPEND SOURCES xpr_crc.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_crc.h)
endif()

if(HAVE_XPR_DEVCAPS)
    list(APPEND SOURCES xpr_devcaps.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_devcaps.h)
endif()

if(HAVE_XPR_DPU)
    list(APPEND SOURCES xpr_dpu.c xpr_dpu_options.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_dpu.h)
endif()

if(HAVE_XPR_DRM)
    list(APPEND SOURCES xpr_drm.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_drm.h)
endif()

if(HAVE_XPR_FIFO)
    list(APPEND SOURCES xpr_fifo.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_fifo.h)
endif()

if(HAVE_XPR_FILE)
    if(UNIX)
        list(APPEND SOURCES xpr_file_unix.c)
    endif()
    if(MSVC)
        list(APPEND SOURCES xpr_file_win32.c)
    endif()
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_file.h)
endif()

if(HAVE_XPR_FQ)
    list(APPEND SOURCES xpr_fq.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_fq.h)
endif()

if(HAVE_XPR_GPIO)
    list(APPEND SOURCES xpr_gpio.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_gpio.h)
endif()

if(HAVE_XPR_H264)
    list(APPEND SOURCES xpr_h264.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_h264.h)
endif()

if(HAVE_XPR_ICMP)
    if(UNIX)
        list(APPEND SOURCES xpr_icmp_unix.c)
    endif()
    if(MSVC)
        list(APPEND SOURCES xpr_icmp_win32.c)
    endif()
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_icmp.h)
endif()

if(HAVE_XPR_JSON)
    list(APPEND SOURCES 
        deps/jansson/dump.c
        deps/jansson/error.c
        deps/jansson/hashtable.c
        deps/jansson/hashtable_seed.c
        deps/jansson/load.c
        deps/jansson/memory.c
        deps/jansson/pack_unpack.c
        deps/jansson/strbuffer.c
        deps/jansson/strconv.c
        deps/jansson/utf.c
        deps/jansson/value.c
        xpr_json.c
    )
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_json.h)
endif()

if(HAVE_XPR_LIST)
    list(APPEND SOURCES xpr_list.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_list.h)
endif()

if(HAVE_XPR_MCDEC)
    list(APPEND SOURCES xpr_mcdec.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_mcdec.h)
endif()

if(HAVE_XPR_MCVR)
    list(APPEND SOURCES xpr_mcvr.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_mcvr.h)
endif()

if(HAVE_XPR_MD5)
    list(APPEND SOURCES xpr_md5.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_md5.h)
endif()

if(HAVE_XPR_MEM)
    list(APPEND SOURCES xpr_mem.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_mem.h)
endif()

if(HAVE_XPR_META)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_meta.h)
endif()

if(HAVE_XPR_PES)
    list(APPEND SOURCES xpr_pes.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_pes.h)
endif()

if(HAVE_XPR_PLUGIN)
    list(APPEND SOURCES xpr_plugin.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_plugin.h)
endif()

if(HAVE_XPR_RTSP_IDD)
    list(APPEND SOURCES xpr_rtsp_idd.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_rtsp_idd.h)
endif()

if(HAVE_XPR_RTSP)
    list(APPEND SOURCES
        drivers/rtsp/live/rtsp.cpp
        drivers/rtsp/live/rtsp_client.cpp
        drivers/rtsp/live/rtsp_connection.cpp
        drivers/rtsp/live/rtsp_connectionmanager.cpp
        drivers/rtsp/live/rtsp_framemerger.cpp
        drivers/rtsp/live/rtsp_server.cpp
        drivers/rtsp/live/rtsp_worker.cpp
        xpr_rtsp.c
    )
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_rtsp.h)
endif()

if(HAVE_XPR_SERIAL)
    if(UNIX)
        list(APPEND SOURCES xpr_serial_unix.c)
    endif()
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_serial.h)
endif()

if(HAVE_XPR_SHA1)
    list(APPEND SOURCES xpr_sha1.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_sha1.h)
endif()

if(HAVE_XPR_STREAMBLOCK)
    list(APPEND SOURCES xpr_streamblock.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_streamblock.h)
endif()

if(HAVE_XPR_SYNC)
    if(UNIX)
        list(APPEND SOURCES xpr_sync_unix.c)
    endif()
    if(MSVC)
        list(APPEND SOURCES xpr_sync_win32.c)
    endif()
    list(APPEND SOURCES xpr_sync.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_sync.h)
endif()

if(HAVE_XPR_SYS)
    if(UNIX)
        list(APPEND SOURCES xpr_sys_unix.c)
    endif()
    if(MSVC)
        list(APPEND SOURCES xpr_sys_win32.c)
    endif()
    list(APPEND SOURCES xpr_sys.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_sys.h)
endif()

if(HAVE_XPR_THREAD)
    if(UNIX)
        list(APPEND SOURCES xpr_thread_unix.c)
    endif()
    if(MSVC)
        list(APPEND SOURCES xpr_thread_win32.c)
    endif()
    list(APPEND SOURCES xpr_thread.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_thread.h)
endif()

if(HAVE_XPR_TIMER)
    if(UNIX)
        list(APPEND SOURCES xpr_timer_unix.c)
    endif()
    if(MSVC)
        list(APPEND SOURCES xpr_timer_win32.c)
    endif()
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_timer.h)
endif()

if(HAVE_XPR_UPS)
    list(APPEND SOURCES xpr_ups.c)
    if(HAVE_XPR_UPS_DRIVER_FAKEROOT)
        list(APPEND SOURCES drivers/ups/fakeroot.c)
    endif()
    if(HAVE_XPR_UPS_DRIVER_SYS_INFO)
        list(APPEND SOURCES drivers/ups/sys_info.c)
    endif()
    if(HAVE_XPR_UPS_DRIVER_SYS_NET)
        list(APPEND SOURCES drivers/ups/sys_net.c)
    endif()
    if(HAVE_XPR_UPS_DRIVER_CAM_IMG)
        list(APPEND SOURCES drivers/ups/cam_img.c)
    endif()
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_ups.h)
endif()

if(HAVE_XPR_URL)
    list(APPEND SOURCES xpr_url.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_url.h)
endif()

if(HAVE_XPR_UTILS)
    list(APPEND SOURCES xpr_utils.c)
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_utils.h)
endif()

if(HAVE_XPR_XML)
    list(APPEND SOURCES
        deps/roxml/roxml_buff.c
        deps/roxml/roxml_content.c 
        deps/roxml/roxml_commit.c
        deps/roxml/roxml_core.c
        deps/roxml/roxml_edit.c 
        deps/roxml/roxml_file.c 
        deps/roxml/roxml_nav.c
        deps/roxml/roxml_mem.c
        deps/roxml/roxml_parser.c
        deps/roxml/roxml_stub.c
        deps/roxml/roxml_xpath.c
        xpr_xml.c
    )
    list(APPEND PUBLIC_HEADERS ${CMAKE_SOURCE_DIR}/include/xpr/xpr_xml.h)
endif()

list(APPEND PUBLIC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/xpr/xpr_atomic.h
    ${CMAKE_SOURCE_DIR}/include/xpr/xpr_common.h
    ${CMAKE_SOURCE_DIR}/include/xpr/xpr_config.h
    ${CMAKE_SOURCE_DIR}/include/xpr/xpr_errno.h
    ${CMAKE_SOURCE_DIR}/include/xpr/xpr.h
)

# Additional definitions
if(UNIX)
    # For: pthread_yield, ...
    add_definitions(-D_GNU_SOURCE)
endif()

# Framework headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Library: libxpr
add_library(${PROJECT_NAME} ${SOURCES})

target_compile_options(${PROJECT_NAME}
    PRIVATE -include ${CMAKE_SOURCE_DIR}/include/xpr/xpr_config.h
)

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        PUBLIC_HEADER "${PUBLIC_HEADERS}"
)

if(BUILD_WITH_HUNTER)
    if(HAVE_XPR_RTSP)
        set(LIVE_LIBRARIES live::live)
    endif()
    target_include_directories(${PROJECT_NAME}
        PUBLIC ${LIVE_INCLUDE_DIRS}
    )
    target_link_libraries(${PROJECT_NAME}
        PUBLIC ${LIVE_LIBRARIES}
    )
endif()

# Installation
#
install(
    TARGETS ${PROJECT_NAME}
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION "lib"
    ARCHIVE DESTINATION "lib"
    RUNTIME DESTINATION "bin"
    INCLUDES DESTINATION "${INCLUDE_INSTALL_DIR}"
    PUBLIC_HEADER DESTINATION "${INCLUDE_INSTALL_DIR}/xpr"
)

install(
    EXPORT "${TARGETS_EXPORT_NAME}"
    NAMESPACE "${NAMESPACE}"
    DESTINATION "${CONFIG_INSTALL_DIR}"
)