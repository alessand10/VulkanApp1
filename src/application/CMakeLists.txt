cmake_minimum_required(VERSION 3.30)

set(CXX_STANDARD C++17)
add_library(application vulkan-app.cpp)
add_compile_definitions("SOURCE_ROOT=${CMAKE_SOURCE_DIR}/build")

target_include_directories(application  PUBLIC ${CMAKE_SOURCE_DIR}/src/application
                                        PUBLIC ${CMAKE_SOURCE_DIR}/src/geometry
                                        PUBLIC ${CMAKE_SOURCE_DIR}/include)
find_package(tinyobjloader REQUIRED)
find_package(VulkanHeaders REQUIRED)
find_package(VulkanLoader REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

target_link_libraries(application   PUBLIC tinyobjloader::tinyobjloader
                                    PUBLIC render
                                    PUBLIC geometry
                                    PUBLIC resources
                                    PUBLIC general-utils
                                    PUBLIC image
                                    Qt6::Core
                                    Qt6::Gui
                                    Qt6::Widgets
                                    )