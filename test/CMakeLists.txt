

add_executable(buffer_IO_check buffer_IO_check.cc)
target_link_libraries(buffer_IO_check pthread ${Boost_PROGRAM_OPTIONS_LIBRARY} )

