include_directories(${CMAKE_SOURCE_DIR}/app-framework/include)
include_directories(${CMAKE_SOURCE_DIR}/app-framework-base/include)

add_executable(simple_test_app simple_test_app.cc)
target_link_libraries(simple_test_app app-framework app-framework_DAQModules pthread ers)

add_executable(producer_consumer_test_app producer_consumer_test_app.cc)
target_link_libraries(producer_consumer_test_app app-framework app-framework_DAQModules pthread ers)