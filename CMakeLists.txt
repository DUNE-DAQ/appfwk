cmake_minimum_required(VERSION 3.12)
project(app-framework-base)

set(CMAKE_CXX_STANDARD 17)

find_package(cetlib REQUIRED)

add_subdirectory(doc)
add_subdirectory(unittest)
