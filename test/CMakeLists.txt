

add_executable(buffer_concurrency_check buffer_concurrency_check.cc)
target_link_libraries(buffer_concurrency_check pthread ${Boost_PROGRAM_OPTIONS_LIBRARY} )

