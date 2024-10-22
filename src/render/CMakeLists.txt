add_library(render camera.cpp render-utilities.cpp)

target_link_libraries(render PUBLIC  general-utils
                                        resources
                                        material)

target_include_directories(render PUBLIC ${CMAKE_SOURCE_DIR}/src/render)