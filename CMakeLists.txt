add_compile_options(-g -lglfw -lGLU -lGL -lm)

# Execute the bash script to compile shaders
execute_process(COMMAND bash ./compileshaders.sh)

set(CXX_STANDARD C++17)


set(VK_LOADER_DEBUG error)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#set(GLFW_USE_WAYLAND ON)

cmake_minimum_required(VERSION 3.10)

project(VulkanApp1)

add_executable(VulkanApp1 src/main.cpp src/vulkan-app.cpp src/camera.cpp src/mesh.cpp src/mesh-manager.cpp src/vertex.cpp 
    src/resource-manager.cpp src/attach.cpp src/raytracing.cpp src/utilities.cpp)


# Specify the directories that we will be looking for static libraries in
target_link_directories(VulkanApp1  PUBLIC "~/Downloads/glfw/build/src"
                                    PUBLIC "~/Downloads/VulkanSDK/1.3.290.0/x86_64/lib"
                                    PUBLIC "~/Downloads/libjpeg-turbo")

# Link the static libraries we will be using
target_link_libraries(VulkanApp1    PUBLIC "libglfw3.a"
                                    PUBLIC "libvulkan.so"
                                    PUBLIC "libturbojpeg.so")

# Specify where the headers will be located
target_include_directories(VulkanApp1   PUBLIC "~/Downloads/glfw/include"
                                        PUBLIC "~/Downloads/VulkanSDK/1.3.290.0/x86_64/include"
                                        PUBLIC ${CMAKE_SOURCE_DIR}/include
                                        PUBLIC "~/Downloads/glm/glm"
                                        PUBLIC "~/Downloads/libjpeg-turbo")
