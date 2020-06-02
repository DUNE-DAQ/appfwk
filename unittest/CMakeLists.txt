
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_NO_BOOST_CMAKE ON)
set(BUILD_SHARED_LIBS ON)
find_package(Boost COMPONENTS unit_test_framework)

if(Boost_FOUND)

    include_directories(${Boost_INCLUDE_DIRS})

    add_executable(DAQProcess_test DAQProcess_test.cc)
    target_link_libraries(DAQProcess_test app-framework ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} ers)
    target_compile_definitions(DAQProcess_test PRIVATE "BOOST_TEST_DYN_LINK=1")
    target_include_directories(DAQProcess_test PRIVATE ${Boost_INCLUDE_DIRS})
    add_test(NAME DAQProcess_test COMMAND DAQProcess_test)
    
    add_executable(StdDeQueue_test StdDeQueue_test.cc)
    add_test(NAME StdDeQueue_test COMMAND StdDeQueue_test)

    add_subdirectory(DAQModules)
endif()

