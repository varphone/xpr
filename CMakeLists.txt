cmake_minimum_required(VERSION 3.2)

# Build options
option(BUILD_INCLUDE        "Build include directory"               OFF)
option(BUILD_SRC            "Build src directory"                   ON)
option(BUILD_DOCS           "Build docs directory"                  OFF)
option(BUILD_EXAMPLES       "Build examples directory"              ON)
option(BUILD_TESTS          "Build tests directory"                 OFF)
option(BUILD_TOOLS          "Build tools directory"                 OFF)
option(BUILD_WITH_HUNTER    "Build with hunter package manager"     OFF)

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
option(HAVE_XPR_FILE         "Enable File module"                    ON)
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

# Import hunter package manager
if(BUILD_WITH_HUNTER)
    include("cmake/HunterGate.cmake")

    HunterGate(
        URL "https://github.com/varphone/hunter/archive/v0.23.59-p1.tar.gz"
        SHA1 "893a82934813dc9a9a788ec0203f782343d96758"
        LOCAL
    )
endif()

# Setup project informations
project(xpr
    VERSION 2.1.9
    LANGUAGES C CXX
)

# Check headers
include(CheckIncludeFiles)
CHECK_INCLUDE_FILES(pthread.h HAVE_PTHREAD_H)
CHECK_INCLUDE_FILES(sched.h HAVE_SCHED_H)
CHECK_INCLUDE_FILES(stdint.h HAVE_STDINT_H)
CHECK_INCLUDE_FILES(time.h HAVE_TIME_H)
CHECK_INCLUDE_FILES(unistd.h HAVE_UNISTD_H)

# Dependency
#
if(BUILD_WITH_HUNTER)
    # live
    hunter_add_package(live)
    find_package(live CONFIG REQUIRED)
    if (NOT LIVE_INCLUDE_DIRS)
        set(LIVE_INCLUDE_DIRS ${LIVE_ROOT}/include)
    endif()
else()
    find_package(live QUIET)

    if(NOT live_FOUND)
        set(HAVE_XPR_RTSP_DRIVER_LIVE OFF)
    endif()
endif()

# Generate config file
configure_file(
    ${CMAKE_SOURCE_DIR}/include/xpr/xpr_config.h.in
    ${CMAKE_SOURCE_DIR}/include/xpr/xpr_config.h
    @ONLY
)

# Installation properties
#
set(CONFIG_INSTALL_DIR "lib/cmake/${PROJECT_NAME}")
set(INCLUDE_INSTALL_DIR "include")

set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")

set(VERSION_CONFIG "${GENERATED_DIR}/${PROJECT_NAME}ConfigVersion.cmake")
set(PROJECT_CONFIG "${GENERATED_DIR}/${PROJECT_NAME}Config.cmake")
set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")
set(NAMESPACE "${PROJECT_NAME}::")

# Subdirectories
#
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

# Installation (https://github.com/forexample/package-example)
#
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${VERSION_CONFIG}" COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "cmake/Config.cmake.in"
    "${PROJECT_CONFIG}"
    INSTALL_DESTINATION "${CONFIG_INSTALL_DIR}"
)

install(
    FILES "${PROJECT_CONFIG}" "${VERSION_CONFIG}"
    DESTINATION "${CONFIG_INSTALL_DIR}"
)
