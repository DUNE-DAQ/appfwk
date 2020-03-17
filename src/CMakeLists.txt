include_directories(${CMAKE_SOURCE_DIR}/app-framework/include)
include_directories(${CMAKE_SOURCE_DIR}/app-framework-base/include)

file(GLOB source_files *.cc)
add_library(app-framework ${source_files})