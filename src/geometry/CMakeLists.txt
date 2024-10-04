add_library(geometry "geometry-utilities.cpp" "geometry-manager.cpp" "mesh.cpp")

target_link_libraries(geometry PUBLIC general-utils)

target_include_directories(geometry PUBLIC ${CMAKE_SOURCE_DIR}/src/geometry)