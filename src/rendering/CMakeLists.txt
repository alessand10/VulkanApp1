add_library(rendering camera.cpp rendering-utilities.cpp)

target_link_libraries(rendering PUBLIC general-utils)

target_include_directories(rendering PUBLIC ${CMAKE_SOURCE_DIR}/src/rendering)