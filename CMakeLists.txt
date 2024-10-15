set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CXX_STANDARD C++17)
cmake_minimum_required(VERSION 3.30)
project(VulkanApp)

add_subdirectory(src/resources)
add_subdirectory(src/geometry)
add_subdirectory(src/general-utils)
add_subdirectory(src/application)
add_subdirectory(src/rendering)

add_executable(VulkanApp src/app-config.cpp)

if (CMAKE_HOST_UNIX)
    if (CMAKE_HOST_APPLE)
        message(STATUS ">>> Apple system detected")
    else()
        message(STATUS ">>> Linux system detected")
        add_compile_options(-g -lglfw -lGLU -lGL -lm)
        add_compile_definitions("SOURCE_ROOT=${CMAKE_SOURCE_DIR}/build" LINUX_PLATFORM)
        find_package(glfw3 REQUIRED)
        find_package(libjpeg-turbo REQUIRED)
        find_package(VulkanHeaders REQUIRED)
        find_package(VulkanLoader REQUIRED)
        find_package(glm REQUIRED)
        target_link_libraries ( 
                                ${PROJECT_NAME} 
                                PUBLIC libjpeg-turbo::libjpeg-turbo
                                PUBLIC glfw
                                PUBLIC vulkan-headers::vulkan-headers
                                PUBLIC Vulkan::Loader
                                PUBLIC glm::glm
                                PUBLIC geometry
                                PUBLIC resources
                                PUBLIC application
                                PUBLIC rendering
                                PUBLIC general-utils
                            )
    endif()
elseif (CMAKE_HOST_WIN32)
    message(STATUS ">>> Windows system detected")
    add_compile_options(-Zi)
    add_compile_definitions("SOURCE_ROOT=${CMAKE_SOURCE_DIR}/build" WINDOWS_PLATFORM)
    find_package(libjpeg-turbo REQUIRED)
    find_package(glfw3 REQUIRED)
    find_package(glm REQUIRED)
    find_package(VulkanLoader REQUIRED)
    find_package(VulkanHeaders REQUIRED)
    target_link_libraries ( 
                            ${PROJECT_NAME} 
                            PUBLIC libjpeg-turbo::libjpeg-turbo
                            PUBLIC glfw
                            PUBLIC vulkan-headers::vulkan-headers
                            PUBLIC Vulkan::Loader
                            PUBLIC glm::glm
                        )
endif()


set(VK_LOADER_DEBUG error)


# Specify where the headers will be located
target_include_directories(VulkanApp PUBLIC ${CMAKE_SOURCE_DIR}/include)
