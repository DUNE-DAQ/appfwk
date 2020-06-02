cmake_minimum_required(VERSION 3.12)
project(app-framework-base)

set(CMAKE_CXX_STANDARD 17)

if(EXISTS $ENV{CETLIB_LIB})
  include_directories($ENV{CETLIB_INC})
else()
	find_package(cetlib REQUIRED)
endif()


include_directories(${CMAKE_SOURCE_DIR}/ers)

add_subdirectory(doc)
add_subdirectory(unittest)
