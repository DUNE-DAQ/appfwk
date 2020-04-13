cmake_minimum_required(VERSION 3.12)
project(app-framework)

set(CMAKE_CXX_STANDARD 17)

find_package(TRACE 3.15.0 REQUIRED)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_NO_BOOST_CMAKE ON)
set(BUILD_SHARED_LIBS ON)
find_package(Boost 1.72.0 COMPONENTS unit_test_framework)

include_directories(${CMAKE_SOURCE_DIR}/app-framework/include)
include_directories(${CMAKE_SOURCE_DIR}/app-framework-base/include)

add_subdirectory(apps)
add_subdirectory(doc)
add_subdirectory(include)
add_subdirectory(scripts)
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(unittest)