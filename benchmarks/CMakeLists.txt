include_directories(${CMAKE_SOURCE_DIR}/include)

if(HAVE_XPR_RTSP)
add_executable(rtsp-conn-open-close benchmark.c rtsp-conn-open-close.c)
target_link_libraries(rtsp-conn-open-close xpr -lpthread)

add_executable(rtsp-conn-rx-264 benchmark.c rtsp-conn-rx-264.c)
target_link_libraries(rtsp-conn-rx-264 xpr -lpthread)

add_executable(rtsp-conn-rx-aac benchmark.c rtsp-conn-rx-aac.c)
target_link_libraries(rtsp-conn-rx-aac xpr -lpthread)

add_executable(rtsp-conn-start-stop benchmark.c rtsp-conn-start-stop.c)
target_link_libraries(rtsp-conn-start-stop xpr -lpthread)

add_executable(rtsp-mgr-open-close benchmark.c rtsp-mgr-open-close.c)
target_link_libraries(rtsp-mgr-open-close xpr -lpthread)

add_executable(rtsp-mgr-start-stop benchmark.c rtsp-mgr-start-stop.c)
target_link_libraries(rtsp-mgr-start-stop xpr -lpthread)

add_executable(rtsp-svr-tx-aac benchmark.c rtsp-svr-tx-aac.c)
target_link_libraries(rtsp-svr-tx-aac xpr -lpthread)
endif()

if(HAVE_XPR_THREAD)
add_executable(thread-create-destroy benchmark.c thread-create-destroy.c)
target_link_libraries(thread-create-destroy xpr -lpthread)
endif()