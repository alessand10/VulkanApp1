cmake_minimum_required(VERSION 3.0)

add_library(material material.cpp material-blueprint.cpp)

target_link_libraries(material  PUBLIC image
                                PUBLIC resources)

target_include_directories(material PUBLIC ${CMAKE_SOURCE_DIR}/src/material
                                    PUBLIC ${CMAKE_SOURCE_DIR}/include)
