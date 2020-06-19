cmake_minimum_required(VERSION 3.12)
project(appfwk)

find_package(cetlib REQUIRED)
find_package(folly REQUIRED)

set(DAQ_LIBRARIES_PACKAGE ${CETLIB} ${CETLIB_EXCEPT} appfwk)

##############################################################################
point_build_to( apps )

add_executable(daq_application apps/daq_application.cxx)
target_link_libraries(daq_application ${DAQ_LIBRARIES_UNIVERSAL_EXE} ${DAQ_LIBRARIES_PACKAGE} Folly::folly)

##############################################################################
point_build_to( doc )
# (No action taken, doc/CMakeLists.txt was empty)

##############################################################################
point_build_to( include )  
set(PATH_DIRS ${CMAKE_CURRENT_BINARY_DIR}/scripts:${CMAKE_CURRENT_BINARY_DIR}/apps:${CMAKE_CURRENT_BINARY_DIR}/test)
set(LIB_DIRS ${CMAKE_CURRENT_BINARY_DIR}/src:${CMAKE_CURRENT_BINARY_DIR}/test)
configure_file(scripts/setupForRunning.sh.in scripts/setupForRunning.sh @ONLY)


##############################################################################
point_build_to( src )

add_library(appfwk SHARED src/QueueRegistry.cpp src/DAQProcess.cpp src/DAQModule.cpp)

set( APPFWK_LIBRARIES ${DAQ_LIBRARIES_PACKAGE} )
list(REMOVE_ITEM APPFWK_LIBRARIES appfwk)
target_link_libraries(appfwk ${APPFWK_LIBRARIES}  Folly::folly)

add_library(appfwk_QueryResponseCommandFacility_duneCommandFacility src/QueryResponseCommandFacility.cpp)

macro(MakeDataTypeLibraries)
   set(oneValueArgs CPPTYPE PREFIX)
   cmake_parse_arguments(MAKE_DATATYPE "" "${oneValueArgs}" "" ${ARGN})

   configure_file(src/FanOutDAQModule.cpp.in src/${MAKE_DATATYPE_PREFIX}FanOutDAQModule.cpp)
   add_library(${MAKE_DATATYPE_PREFIX}FanOutDAQModule_duneDAQModule SHARED src/${MAKE_DATATYPE_PREFIX}FanOutDAQModule.cpp)
   target_link_libraries(${MAKE_DATATYPE_PREFIX}FanOutDAQModule_duneDAQModule Folly::folly)

endmacro()

MakeDataTypeLibraries(CPPTYPE std::vector<int> PREFIX VectorInt)
MakeDataTypeLibraries(CPPTYPE std::string PREFIX String)
MakeDataTypeLibraries(CPPTYPE dunedaq::appfwk::NonCopyableType PREFIX NonCopyableType)

##############################################################################
point_build_to( test )

add_executable(queue_IO_check test/queue_IO_check.cxx)
target_link_libraries(queue_IO_check ${DAQ_LIBRARIES_UNIVERSAL_EXE} Folly::folly)
add_executable(dummy_test_app test/dummy_test_app.cxx)
target_link_libraries(dummy_test_app appfwk appfwk_DummyModule_duneDAQModule pthread ers  ${CETLIB} ${CETLIB_EXCEPT} ${Boost_PROGRAM_OPTIONS_LIBRARY})



add_library(appfwk_DummyModule_duneDAQModule test/DummyModule.cpp)
add_library(appfwk_DebugLoggingDAQModule_duneDAQModule test/DebugLoggingDAQModule.cpp)
target_link_libraries(appfwk_DebugLoggingDAQModule_duneDAQModule Folly::folly)
add_library(appfwk_FakeDataConsumerDAQModule_duneDAQModule test/FakeDataConsumerDAQModule.cpp)
target_link_libraries(appfwk_FakeDataConsumerDAQModule_duneDAQModule Folly::folly)
add_library(appfwk_FakeDataProducerDAQModule_duneDAQModule test/FakeDataProducerDAQModule.cpp)
target_link_libraries(appfwk_FakeDataProducerDAQModule_duneDAQModule Folly::folly)

# Curious to hear people's take on this format style which aligns the target across lines...
add_executable        (echo_test_app test/echo_test_app.cxx)
target_link_libraries (echo_test_app ${DAQ_LIBRARIES_UNIVERSAL_EXE} ${DAQ_LIBRARIES_PACKAGE} appfwk_DebugLoggingDAQModule_duneDAQModule  Folly::folly)

file(COPY test/producer_consumer_dynamic_test.json DESTINATION test)

##############################################################################
point_build_to( unittest )

add_unit_test(ThreadHelper_test)
add_unit_test(DAQSink_DAQSource_test)
add_unit_test(StdDeQueue_test)
add_unit_test(FollyQueue_test)
# add_unit_test(FanOutDAQModule_test)

