cmake_minimum_required(VERSION 3.12)
project(appfwk)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../daq-buildtools/cmake ${CMAKE_MODULE_PATH})
include(DAQ)

daq_setup_environment()

find_package(Boost 1.70.0 COMPONENTS unit_test_framework program_options REQUIRED)
find_package(TRACE 3.15.09 REQUIRED)
find_package(cetlib REQUIRED)   # Uses the daq-buildtools/cmake/Findcetlib.cmake
find_package(folly REQUIRED)
find_package(ers REQUIRED)

find_package(nlohmann_json 3.8.0 )

if(NOT ${nlohmann_json_FOUND})
  message("nlohmann_json NOT FOUND! Downloading single-header from GitHub!")
  file(DOWNLOAD https://github.com/nlohmann/json/raw/v3.8.0/single_include/nlohmann/json.hpp nlohmann/json.hpp)
  include_directories(${CMAKE_BINARY_DIR})
endif()

set(APPFWK_DEPENDENCIES ${CETLIB} ${CETLIB_EXCEPT} ers::ers Folly::folly pthread)

##############################################################################
point_build_to( src )

add_library(appfwk SHARED src/QueueRegistry.cpp src/DAQProcess.cpp src/DAQModule.cpp)
target_link_libraries(appfwk ${APPFWK_DEPENDENCIES})
target_include_directories(appfwk PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include> $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}> )

add_library(appfwk_QueryResponseCommandFacility_duneCommandFacility src/QueryResponseCommandFacility.cpp)
target_link_libraries(appfwk_QueryResponseCommandFacility_duneCommandFacility appfwk)

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
point_build_to( apps )

add_executable(daq_application apps/daq_application.cxx)
target_link_libraries(daq_application ${Boost_PROGRAM_OPTIONS_LIBRARY} appfwk)

##############################################################################
point_build_to( doc )
# (No action taken, doc/ doesn't yet contain anything)

##############################################################################
point_build_to( test )

add_library(appfwk_DummyModule_duneDAQModule test/DummyModule.cpp)
target_link_libraries(appfwk_DummyModule_duneDAQModule appfwk)

add_library(appfwk_FakeDataConsumerDAQModule_duneDAQModule test/FakeDataConsumerDAQModule.cpp)
target_link_libraries(appfwk_FakeDataConsumerDAQModule_duneDAQModule appfwk)

add_library(appfwk_FakeDataProducerDAQModule_duneDAQModule test/FakeDataProducerDAQModule.cpp)
target_link_libraries(appfwk_FakeDataProducerDAQModule_duneDAQModule appfwk)

add_executable(queue_IO_check test/queue_IO_check.cxx)
target_link_libraries(queue_IO_check ${Boost_PROGRAM_OPTIONS_LIBRARY} appfwk)

add_executable(dummy_test_app test/dummy_test_app.cxx)
target_link_libraries(dummy_test_app appfwk appfwk_DummyModule_duneDAQModule ${Boost_PROGRAM_OPTIONS_LIBRARY} appfwk)


file(COPY test/producer_consumer_dynamic_test.json DESTINATION test)

##############################################################################
point_build_to( unittest )

add_unit_test(DAQModule_test          appfwk )
add_unit_test(DAQSink_DAQSource_test  appfwk )
add_unit_test(FanOutDAQModule_test    appfwk )
add_unit_test(FollyQueue_test         ers::ers Folly::folly)
add_unit_test(StdDeQueue_test         ers::ers)
add_unit_test(ThreadHelper_test       ers::ers)

daq_install( VERSION 1.0.0 
	     TARGETS daq_application 
                     appfwk 
                     VectorIntFanOutDAQModule_duneDAQModule StringFanOutDAQModule_duneDAQModule NonCopyableTypeFanOutDAQModule_duneDAQModule
	   )
