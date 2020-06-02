if(Boost_FOUND)

    include_directories(${Boost_INCLUDE_DIRS})

    add_executable(StdDeQueue_test StdDeQueue_test.cc)
    add_test(NAME StdDeQueue_test COMMAND StdDeQueue_test)

    add_subdirectory(DAQModules)
endif()

