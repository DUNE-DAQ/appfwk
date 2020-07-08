cmake_minimum_required(VERSION 3.12)
project(appfwk)

set(DAQ_LIBRARIES_PACKAGE ${CETLIB} ${CETLIB_EXCEPT} Folly::folly)

##############################################################################
point_build_to( apps )

add_executable(daq_application apps/daq_application.cxx)
target_link_libraries(daq_application ${DAQ_LIBRARIES_UNIVERSAL_EXE} appfwk)

##############################################################################
point_build_to( doc )
# (No action taken, doc/CMakeLists.txt was empty)

##############################################################################
point_build_to( src )

add_library(appfwk SHARED src/QueueRegistry.cpp src/DAQProcess.cpp src/DAQModule.cpp)

target_link_libraries(appfwk ${DAQ_LIBRARIES_PACKAGE})

add_library(appfwk_QueryResponseCommandFacility_duneCommandFacility src/QueryResponseCommandFacility.cpp)

macro(MakeDataTypeLibraries)
   set(oneValueArgs CPPTYPE PREFIX)
   cmake_parse_arguments(MAKE_DATATYPE "" "${oneValueArgs}" "" ${ARGN})

   configure_file(src/FanOutDAQModule.cpp.in src/${MAKE_DATATYPE_PREFIX}FanOutDAQModule.cpp)
   add_library(${MAKE_DATATYPE_PREFIX}FanOutDAQModule_duneDAQModule SHARED src/${MAKE_DATATYPE_PREFIX}FanOutDAQModule.cpp)
   target_link_libraries(${MAKE_DATATYPE_PREFIX}FanOutDAQModule_duneDAQModule appfwk)

endmacro()

MakeDataTypeLibraries(CPPTYPE std::vector<int> PREFIX VectorInt)
MakeDataTypeLibraries(CPPTYPE std::string PREFIX String)
MakeDataTypeLibraries(CPPTYPE dunedaq::appfwk::NonCopyableType PREFIX NonCopyableType)

##############################################################################
point_build_to( test )

add_library(appfwk_DummyModule_duneDAQModule test/DummyModule.cpp)

add_library(appfwk_FakeDataConsumerDAQModule_duneDAQModule test/FakeDataConsumerDAQModule.cpp)
target_link_libraries(appfwk_FakeDataConsumerDAQModule_duneDAQModule appfwk)

add_library(appfwk_FakeDataProducerDAQModule_duneDAQModule test/FakeDataProducerDAQModule.cpp)
target_link_libraries(appfwk_FakeDataProducerDAQModule_duneDAQModule appfwk)


add_executable(queue_IO_check test/queue_IO_check.cxx)
target_link_libraries(queue_IO_check ${DAQ_LIBRARIES_UNIVERSAL_EXE} appfwk)

add_executable(dummy_test_app test/dummy_test_app.cxx)
target_link_libraries(dummy_test_app appfwk appfwk_DummyModule_duneDAQModule ${DAQ_LIBRARIES_UNIVERSAL_EXE} appfwk)


file(COPY test/producer_consumer_dynamic_test.json DESTINATION test)

##############################################################################
point_build_to( unittest )

add_unit_test(DAQModule_test          appfwk )
add_unit_test(DAQSink_DAQSource_test  appfwk )
add_unit_test(FanOutDAQModule_test    appfwk )
add_unit_test(FollyQueue_test         appfwk )
add_unit_test(StdDeQueue_test         appfwk )
add_unit_test(ThreadHelper_test       appfwk )


