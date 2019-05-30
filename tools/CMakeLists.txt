include_directories(${CMAKE_SOURCE_DIR}/include)

add_executable(unframed unframed.c)
target_link_libraries(unframed xpr -lpthread)
