add_library(resources   app-resources/buffer-resource.cpp
                        app-resources/command-pool-resource.cpp
                        app-resources/descriptor-pool-resource.cpp
                        app-resources/descriptor-set-layout-resource.cpp
                        app-resources/device-memory-resource.cpp
                        app-resources/device-resource.cpp
                        app-resources/fence-resource.cpp
                        app-resources/framebuffer-resource.cpp
                        app-resources/image-resource.cpp
                        app-resources/image-view-resource.cpp
                        app-resources/instance-resource.cpp
                        app-resources/pipeline-layout-resource.cpp
                        app-resources/pipeline-resource.cpp
                        app-resources/render-pass-resource.cpp
                        app-resources/sampler-resource.cpp
                        app-resources/semaphore-resource.cpp
                        app-resources/shader-module-resource.cpp
                        app-resources/surface-resource.cpp
                        app-resources/swapchain-resource.cpp
                    )

target_link_libraries(resources PUBLIC application
                                PUBLIC rendering
                                PUBLIC geometry)

target_include_directories(resources    PUBLIC ${CMAKE_SOURCE_DIR}/src/resources
                                        PUBLIC ${CMAKE_SOURCE_DIR}/src/resources/lists
                                        PUBLIC ${CMAKE_SOURCE_DIR}/src/resources/app-resources
                                        PUBLIC ${CMAKE_SOURCE_DIR}/include)