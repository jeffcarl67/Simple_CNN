cmake_minimum_required(VERSION 3.1)

project(CNN)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
add_subdirectory(src)
add_subdirectory(lib)
add_subdirectory(test)