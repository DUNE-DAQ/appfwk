cmake_minimum_required(VERSION 3.12)
project(app-framework-base)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/ers)

add_subdirectory(doc)
add_subdirectory(unittest)
