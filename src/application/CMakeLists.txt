cmake_minimum_required(VERSION 3.30)

add_library(application vulkan-app.cpp)

target_include_directories(application  PUBLIC ${CMAKE_SOURCE_DIR}/src/application
                                        PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(application   PRIVATE rendering
                                    PRIVATE geometry
                                    PRIVATE resources
                                    PRIVATE general-utils
                                    )