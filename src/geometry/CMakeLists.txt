add_library(geometry "geometry-utilities.cpp" "mesh.cpp" "geometry-manager.cpp" "geometry-base.cpp")

find_package(tinyobjloader REQUIRED)

target_link_libraries(geometry PUBLIC   general-utils
                                        resources
                                        tinyobjloader::tinyobjloader
                                        material)

target_include_directories(geometry PUBLIC ${CMAKE_SOURCE_DIR}/src/geometry
                                    PUBLIC ${CMAKE_SOURCE_DIR}/src/memory
                                    PUBLIC ${CMAKE_SOURCE_DIR}/src)