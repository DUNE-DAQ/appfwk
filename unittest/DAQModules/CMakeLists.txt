
add_executable(DAQModuleThreadHelper_test DAQModuleThreadHelper_test.cc)
target_link_libraries(DAQModuleThreadHelper_test ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} pthread)
target_compile_definitions(DAQModuleThreadHelper_test PRIVATE "BOOST_TEST_DYN_LINK=1")
target_include_directories(DAQModuleThreadHelper_test PRIVATE ${Boost_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/app-framework-base/include)
add_test(NAME DAQModuleThreadHelper_test COMMAND DAQModuleThreadHelper_test)



