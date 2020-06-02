cmake_minimum_required(VERSION 3.12)
project(app-framework)

set(CMAKE_CXX_STANDARD 17)

find_package(TRACE 3.15.0 REQUIRED)

find_package(nlohmann_json 3.2.0)
if(NOT ${nlohmann_json_FOUND})
  message("nlohmann_json NOT FOUND! Downloading single-header from GitHub!")
  file(DOWNLOAD https://github.com/nlohmann/json/raw/develop/single_include/nlohmann/json.hpp nlohmann/json.hpp)
  include_directories(${CMAKE_BINARY_DIR})
endif()

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_NO_BOOST_CMAKE ON)
set(BUILD_SHARED_LIBS ON)
find_package(Boost 1.70.0 COMPONENTS unit_test_framework program_options REQUIRED)


if(EXISTS $ENV{CETLIB_LIB})
# UPS
  include_directories($ENV{CETLIB_INC})
  include_directories($ENV{CETLIB_EXCEPT_INC})
  find_library(CETLIB NAMES libcetlib.so)
  find_library(CETLIB_EXCEPT NAMES libcetlib_except.so)
else()
	# Spack
	find_package(cetlib REQUIRED)
	set(CETLIB cetlib)
	set(CETLIB_EXCEPT cetlib_except)
endif()

include_directories(${CMAKE_SOURCE_DIR}/app-framework/include)
include_directories(${CMAKE_SOURCE_DIR}/app-framework-base/include)
include_directories(${CMAKE_SOURCE_DIR}/ers)

add_subdirectory(apps)
add_subdirectory(doc)
add_subdirectory(include)
add_subdirectory(scripts)
add_subdirectory(src)
add_subdirectory(test)
add_subdirectory(unittest)