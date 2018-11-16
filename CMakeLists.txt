cmake_minimum_required(VERSION 3.2)

# Build options
option(BUILD_INCLUDE    "Build include directory"               OFF)
option(BUILD_SRC        "Build src directory"                   ON)
option(BUILD_DOCS       "Build docs directory"                  OFF)
option(BUILD_EXAMPLES   "Build examples directory"              ON)
option(BUILD_TESTS      "Build tests directory"                 OFF)
option(BUILD_TOOLS      "Build tools directory"                 OFF)

# Module options
option(HAVE_XPR_ADC          "Enable ADC module"                     OFF)
option(HAVE_XPR_ARP          "Enable ARP module"                     OFF)
option(HAVE_XPR_ARR          "Enable Audio Rate Resample module"     OFF)
option(HAVE_XPR_AVFRAME      "Enable AVFrame module"                 ON)
option(HAVE_XPR_BASE64       "Enable Base64 module"                  ON)
option(HAVE_XPR_BITVECTOR    "Enable BitVector module"               ON)
option(HAVE_XPR_CRC          "Enable CRC module"                     ON)
option(HAVE_XPR_DEVCAPS      "Enable Device Capabilities module"     OFF)
option(HAVE_XPR_DPU          "Enable Data Process Unit module"       OFF)
option(HAVE_XPR_DRM          "Enable Digital Right Manage module"    OFF)
option(HAVE_XPR_FIFO         "Enable Fifo module"                    ON)
option(HAVE_XPR_FILE         "Enable File module"                    OFF)
option(HAVE_XPR_FQ           "Enable Flex Queue module"              ON)
option(HAVE_XPR_GPIO         "Enable GPIO module"                    OFF)
option(HAVE_XPR_H264         "Enable H264 Helper module"             ON)
option(HAVE_XPR_ICMP         "Enable ICMP Helper module"             OFF)
option(HAVE_XPR_JSON         "Enable Json module"                    OFF)
option(HAVE_XPR_MCDEC        "Enable Multi-Chn Decoder module"       OFF)
option(HAVE_XPR_MCVR         "Enable Multi-Chn Video Redner module"  OFF)
option(HAVE_XPR_MD5          "Enable MD5 module"                     ON)
option(HAVE_XPR_MEM          "Enable Memory module"                  ON)
option(HAVE_XPR_PES          "Enable MPEG_PS module"                 ON)
option(HAVE_XPR_PLUGIN       "Enable Plugin Framework module"        OFF)
option(HAVE_XPR_RTSP_IDD     "Enable RTSP-IDD module"                ON)
option(HAVE_XPR_RTSP         "Enable RTSP module"                    ON)
option(HAVE_XPR_RTSP_DRIVER_LIVE  "Use RTSP Live555 impl"            ON)
option(HAVE_XPR_RTSP_SERVER  "Enable RTSP Server module"             ON)
option(HAVE_XPR_SERIAL       "Enable Serial Port module"             OFF)
option(HAVE_XPR_SHA1         "Enable SHA1 module"                    ON)
option(HAVE_XPR_STREAMBLOCK  "Enable Stream Block module"            ON)
option(HAVE_XPR_SYNC         "Enable Synchronization module"         ON)
option(HAVE_XPR_SYS          "Enable System module"                  ON)
option(HAVE_XPR_THREAD       "Enable Thread module"                  ON)
option(HAVE_XPR_UPS          "Enable UPS module"                     OFF)
option(HAVE_XPR_URL          "Enable URL module"                     ON)
option(HAVE_XPR_UTILS        "Enable Utilities module"               ON)
option(HAVE_XPR_XML          "Enable Utilities module"               OFF)

# Setup project informations
project(xpr
    VERSION 1.2.3
    LANGUAGES C CXX
)

# Check headers
include(CheckIncludeFiles)
CHECK_INCLUDE_FILES(pthread.h HAVE_PTHREAD_H)
CHECK_INCLUDE_FILES(sched.h HAVE_SCHED_H)
CHECK_INCLUDE_FILES(time.h HAVE_TIME_H)
CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)

# Generate config file
configure_file(
    ${CMAKE_SOURCE_DIR}/include/xpr/xpr_config.h.in
    ${CMAKE_SOURCE_DIR}/include/xpr/xpr_config.h
    @ONLY
)

# Subdirectories
if(BUILD_INCLUDE)
	add_subdirectory(include)
endif()
if(BUILD_SRC)
	add_subdirectory(src)
endif()
if(BUILD_DOCS)
	add_subdirectory(docs)
endif()
if(BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
if(BUILD_TESTS)
	add_subdirectory(tests)
endif()
if(BUILD_TOOLS)
	add_subdirectory(tools)
endif()