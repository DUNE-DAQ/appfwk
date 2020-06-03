if(Boost_FOUND)

    include_directories(${Boost_INCLUDE_DIRS})

    add_executable(StdDeQueue_test StdDeQueue_test.cc)
    add_test(NAME StdDeQueue_test COMMAND StdDeQueue_test)

        add_executable(FanOutDAQModule_test FanOutDAQModule_test.cc)
    target_link_libraries(FanOutDAQModule_test app-framework ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} pthread ers)
target_compile_definitions(FanOutDAQModule_test PRIVATE "BOOST_TEST_DYN_LINK=1")
    target_include_directories(FanOutDAQModule_test PRIVATE ${Boost_INCLUDE_DIRS})
    add_test(NAME FanOutDAQModule_test COMMAND FanOutDAQModule_test ers)

add_executable(DAQModuleThreadHelper_test DAQModuleThreadHelper_test.cc)
target_link_libraries(DAQModuleThreadHelper_test ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} pthread)
target_compile_definitions(DAQModuleThreadHelper_test PRIVATE "BOOST_TEST_DYN_LINK=1")
target_include_directories(DAQModuleThreadHelper_test PRIVATE ${Boost_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/app-framework-base/include)
add_test(NAME DAQModuleThreadHelper_test COMMAND DAQModuleThreadHelper_test)

endif()

