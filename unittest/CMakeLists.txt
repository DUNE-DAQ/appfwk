

if(Boost_FOUND)
    add_executable(DAQProcess_test DAQProcess_test.cc)
    target_link_libraries(DAQProcess_test app-framework ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY})
target_compile_definitions(DAQProcess_test PRIVATE "BOOST_TEST_DYN_LINK=1")
    target_include_directories(DAQProcess_test PRIVATE ${Boost_INCLUDE_DIRS})
    add_test(NAME DAQProcess_test COMMAND DAQProcess_test)

    add_subdirectory(UserModules)
endif()

