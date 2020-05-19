

if(Boost_FOUND)

    include_directories(${Boost_INCLUDE_DIRS})

    add_executable(DAQProcess_test DAQProcess_test.cc)
    target_link_libraries(DAQProcess_test app-framework ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
target_compile_definitions(DAQProcess_test PRIVATE "BOOST_TEST_DYN_LINK=1")
    add_test(NAME DAQProcess_test COMMAND DAQProcess_test)

    add_executable(DequeBuffer_test DequeBuffer_test.cc)
    add_test(NAME DequeBuffer_test COMMAND DequeBuffer_test)

    add_subdirectory(UserModules)
endif()

