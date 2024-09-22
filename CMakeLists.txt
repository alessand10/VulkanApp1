set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CXX_STANDARD C++17)
cmake_minimum_required(VERSION 3.30)
project(VulkanApp)

add_executable(VulkanApp src/main.cpp src/vulkan-app.cpp src/camera.cpp src/mesh.cpp src/mesh-manager.cpp src/vertex.cpp 
    src/resource-manager.cpp src/attach.cpp src/raytracing.cpp src/utilities.cpp)

if (CMAKE_HOST_UNIX)
    if (CMAKE_HOST_APPLE)
        message(STATUS ">>> Apple system detected")
    else()
        message(STATUS ">>> Linux system detected")
        add_compile_options(-g -lglfw -lGLU -lGL -lm)
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
                            )
    endif()
elseif (CMAKE_HOST_WIN32)
    message(STATUS ">>> Windows system detected")
endif()


set(VK_LOADER_DEBUG error)


# Specify where the headers will be located
target_include_directories(VulkanApp PUBLIC ${CMAKE_SOURCE_DIR}/include)
