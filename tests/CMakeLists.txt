include_directories(${CMAKE_SOURCE_DIR}/include)

if(HAVE_XPR_FIFO)
add_executable(fifo fifo.c)
target_link_libraries(fifo xpr -lpthread)
endif()

if(HAVE_XPR_FQ)
add_executable(fq fq.c)
target_link_libraries(fq xpr -lpthread)
endif()

if(HAVE_XPR_JSON)
add_executable(json json.c)
target_link_libraries(json xpr)
endif()

if(HAVE_XPR_MCDEC)
add_executable(mcdec mcdec.c)
target_link_libraries(mcdec xpr)
endif()

if(HAVE_XPR_QSORT)
add_executable(qsort qsort.c)
target_link_libraries(qsort xpr)
endif()

if(HAVE_XPR_REALPATH)
add_executable(realpath realpath.c)
target_link_libraries(realpath xpr)
endif()

if(HAVE_XPR_RTSP)
add_executable(rtsp rtsp.c)
target_link_libraries(rtsp xpr -lpthread)
endif()

if(HAVE_XPR_RTSP_CLIENT)
add_executable(rtsp_client rtsp_client.c)
target_link_libraries(rtsp_client xpr -lpthread)
endif()

if(HAVE_XPR_STRUCT)
add_executable(struct struct.c)
target_link_libraries(struct xpr)
endif()

if(HAVE_XPR_THREAD)
add_executable(thread thread.c)
target_link_libraries(thread xpr -lpthread)
endif()

if(HAVE_XPR_UPS)
add_executable(ups ups.c)
target_link_libraries(ups xpr)
endif()

if(HAVE_XPR_UTILS)
add_executable(utils utils.c)
target_link_libraries(utils xpr)
endif()

if(HAVE_XPR_XML)
add_executable(xml xml.c)
target_link_libraries(xml xpr)
endif()

