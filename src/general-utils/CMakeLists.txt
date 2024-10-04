add_library(general-utils "file-utilities.cpp" "string-utilities.cpp" "math-utilities.cpp")

target_include_directories(general-utils PUBLIC ${CMAKE_SOURCE_DIR}/src/general-utils)