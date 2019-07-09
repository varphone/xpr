include_directories(${CMAKE_SOURCE_DIR}/include)

list(APPEND CMAKE_SHARED_LINKER_FLAGS -rdynamic)

add_library(plugin_a plugin_a.c)
target_link_libraries(plugin_a xpr)

add_library(plugin_b plugin_b.c)
target_link_libraries(plugin_b xpr)

add_library(plugin_c plugin_c.c)
target_link_libraries(plugin_c xpr)

add_library(plugin_d plugin_d.c)
target_link_libraries(plugin_d xpr)

add_library(plugin_e plugin_e.c)
target_link_libraries(plugin_e xpr)

add_library(plugin_f plugin_f.c)
target_link_libraries(plugin_f xpr)

add_library(plugin_g plugin_g.c)
target_link_libraries(plugin_g xpr)
