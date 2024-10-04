add_library(resources resource-manager.cpp resource-utilities.cpp)

target_link_libraries(resources PUBLIC application
                                PUBLIC rendering
                                PUBLIC geometry)

target_include_directories(resources    PUBLIC ${CMAKE_SOURCE_DIR}/src/resources
                                        PUBLIC ${CMAKE_SOURCE_DIR}/src/resources/lists
                                        PUBLIC ${CMAKE_SOURCE_DIR}/src/resources/app-resources
                                        PUBLIC ${CMAKE_SOURCE_DIR}/include)