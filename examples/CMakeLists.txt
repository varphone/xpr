include_directories(${CMAKE_SOURCE_DIR}/include)

if(HAVE_XPR_PLUGIN)
    add_executable(plugin_framework plugin_framework.c)
    target_link_libraries(plugin_framework xpr)

    # Plugins must build with shared libs
    if(BUILD_SHARED_LIBS)
    add_subdirectory(plugins)
    endif()
endif()
