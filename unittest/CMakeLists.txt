

if(Boost_FOUND)

    include_directories(${Boost_INCLUDE_DIRS})

    add_executable(DAQProcess_test DAQProcess_test.cc)
    target_link_libraries(DAQProcess_test app-framework ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
target_compile_definitions(DAQProcess_test PRIVATE "BOOST_TEST_DYN_LINK=1")
    add_test(NAME DAQProcess_test COMMAND DAQProcess_test)

    add_executable(StdDeQueue_test StdDeQueue_test.cc)
    add_test(NAME StdDeQueue_test COMMAND StdDeQueue_test)

    add_subdirectory(DAQModules)
endif()

