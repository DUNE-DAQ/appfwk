cmake_minimum_required(VERSION 3.12)
project(appfwk VERSION 1.1.0)

find_package(daq-cmake REQUIRED )

daq_setup_environment()

find_package(Boost 1.70.0 COMPONENTS unit_test_framework program_options REQUIRED)
find_package(TRACE 3.15.09 REQUIRED)
find_package(cetlib REQUIRED)   # Uses the daq-buildtools/cmake/Findcetlib.cmake
find_package(folly REQUIRED)
find_package(ers REQUIRED)
find_package(cmdlib REQUIRED)
find_package(nlohmann_json REQUIRED )

set(APPFWK_DEPENDENCIES ${CETLIB} ${CETLIB_EXCEPT} ers::ers Folly::folly cmdlib::cmdlib nlohmann_json::nlohmann_json pthread)

##############################################################################
# Main library
daq_add_library(QueueRegistry.cpp DAQModule*.cpp LINK_LIBRARIES ${APPFWK_DEPENDENCIES})

# ##############################################################################
# Applications
daq_add_application( daq_application daq_application.cxx LINK_LIBRARIES appfwk )

# ##############################################################################
# Test plugins
daq_add_plugin( DummyModule		  duneDAQModule TEST LINK_LIBRARIES appfwk )
daq_add_plugin( FakeDataConsumerDAQModule duneDAQModule TEST LINK_LIBRARIES appfwk )
daq_add_plugin( FakeDataProducerDAQModule duneDAQModule TEST LINK_LIBRARIES appfwk )

# Test applications
daq_add_application( queue_IO_check queue_IO_check.cxx TEST LINK_LIBRARIES appfwk )

# Test configurations
file(COPY test/producer_consumer_dynamic_test.json DESTINATION test)
file(COPY test/test-config.json                    DESTINATION test)

# ##############################################################################
# Unit tests

daq_add_unit_test(DAQModule_test          LINK_LIBRARIES appfwk )
daq_add_unit_test(DAQSink_DAQSource_test  LINK_LIBRARIES appfwk )
daq_add_unit_test(FanOutDAQModule_test    LINK_LIBRARIES appfwk )
daq_add_unit_test(FollyQueue_test         LINK_LIBRARIES ers::ers Folly::folly)
daq_add_unit_test(StdDeQueue_test         LINK_LIBRARIES ers::ers)
daq_add_unit_test(ThreadHelper_test       LINK_LIBRARIES ers::ers)
daq_add_unit_test(NamedObject_test        )

##############################################################################

daq_install()
