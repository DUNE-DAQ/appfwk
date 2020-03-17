find_package(Boost COMPONENTS unit_test_framework)

if(${Boost_FOUND})
    add_executable(DAQProcess_test DAQProcess_test.cc)
    target_link_libraries(DAQProcess_test app-framework)
endif()

