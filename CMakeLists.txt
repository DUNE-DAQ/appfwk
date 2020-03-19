cmake_minimum_required(VERSION 3.12)
project(app-framework)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR}/app-framework/include)
include_directories(${CMAKE_SOURCE_DIR}/app-framework-base/include)

add_subdirectory(apps)
add_subdirectory(doc)
add_subdirectory(include)
add_subdirectory(scripts)
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(unittest)