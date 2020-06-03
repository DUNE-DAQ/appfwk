include_directories(${CMAKE_SOURCE_DIR}/app-framework/include)
include_directories(${CMAKE_SOURCE_DIR}/app-framework-base/include)

add_executable(daq_application daq_application.cc)
target_link_libraries(daq_application app-framework  pthread ers ${CETLIB} ${CETLIB_EXCEPT} ${Boost_PROGRAM_OPTIONS_LIBRARY})

