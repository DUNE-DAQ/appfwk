
set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost COMPONENTS program_options REQUIRED)

add_executable(queue_IO_check queue_IO_check.cc)
target_link_libraries(queue_IO_check pthread ${Boost_PROGRAM_OPTIONS_LIBRARY} )

add_executable(zeromqueue_check zeromqueue_check.cc)
target_link_libraries(zeromqueue_check pthread ${LIBZMQ})

add_executable(zeromqueue_ptr_check zeromqueue_ptr_check.cc)
target_link_libraries(zeromqueue_ptr_check pthread ${LIBZMQ})


