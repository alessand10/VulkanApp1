cmake_minimum_required(VERSION 3.30)

add_library(image image.cpp image-loader.cpp)

target_link_libraries(image PRIVATE libjpeg-turbo::libjpeg-turbo)

target_include_directories(image    PUBLIC ${CMAKE_SOURCE_DIR}/src/image
                                    PUBLIC ${CMAKE_SOURCE_DIR}/src/file)