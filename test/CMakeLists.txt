
set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost COMPONENTS program_options REQUIRED)

add_executable(queue_IO_check queue_IO_check.cc)
target_link_libraries(queue_IO_check pthread ${Boost_PROGRAM_OPTIONS_LIBRARY} )


add_library(app-framework_DebugLoggingDAQModule_duneDAQModule DebugLoggingDAQModule.cc)
add_library(app-framework_FakeDataConsumerDAQModule_duneDAQModule FakeDataConsumerDAQModule.cc)
add_library(app-framework_FakeDataProducerDAQModule_duneDAQModule FakeDataProducerDAQModule.cc)

add_executable(echo_test_app echo_test_app.cc)
target_link_libraries(echo_test_app app-framework app-framework_DebugLoggingDAQModule_duneDAQModule pthread ers  ${CETLIB} ${CETLIB_EXCEPT} ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable(producer_consumer_test_app producer_consumer_test_app.cc)
target_link_libraries(producer_consumer_test_app app-framework app-framework_FakeDataProducerDAQModule_duneDAQModule ers app-framework_FakeDataConsumerDAQModule_duneDAQModule pthread  ${CETLIB} ${CETLIB_EXCEPT} ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable(daq_sink_source daq_sink_source.cc)
target_link_libraries(daq_sink_source app-framework pthread ers)