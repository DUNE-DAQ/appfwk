cmake_minimum_required(VERSION 3.12)
project(appfwk VERSION 2.3.2)

find_package(daq-cmake REQUIRED )

daq_setup_environment()

find_package(Boost 1.70.0 COMPONENTS unit_test_framework program_options REQUIRED)
find_package(TRACE 3.15.09 REQUIRED)
find_package(logging REQUIRED)
find_package(cetlib REQUIRED)   # Uses the daq-buildtools/cmake/Findcetlib.cmake
find_package(folly REQUIRED)
find_package(ers REQUIRED)
find_package(cmdlib REQUIRED)
find_package(rcif REQUIRED)
find_package(networkmanager REQUIRED)
find_package(opmonlib REQUIRED)
find_package(nlohmann_json REQUIRED )

#set(APPFWK_DEPENDENCIES ${CETLIB} ${CETLIB_EXCEPT} ers::ers logging::logging Folly::folly cmdlib::cmdlib rcif::rcif networkmanager::networkmanager opmonlib::opmonlib nlohmann_json::nlohmann_json pthread)
set(APPFWK_DEPENDENCIES ${CETLIB} ${CETLIB_EXCEPT} ers::ers logging::logging Folly::folly cmdlib::cmdlib rcif::rcif networkmanager::networkmanager opmonlib::opmonlib nlohmann_json::nlohmann_json)


daq_codegen( app.jsonnet cmd.jsonnet DEP_PKGS networkmanager rcif cmdlib TEMPLATES Structs.hpp.j2 Nljs.hpp.j2 )
daq_codegen( appinfo.jsonnet queueinfo.jsonnet DEP_PKGS opmonlib TEMPLATES opmonlib/InfoStructs.hpp.j2 opmonlib/InfoNljs.hpp.j2 )

##############################################################################
# Main library
daq_add_library(QueueRegistry.cpp DAQModule*.cpp Application.cpp LINK_LIBRARIES ${APPFWK_DEPENDENCIES})

# ##############################################################################
# Applications
daq_add_application( daq_application daq_application.cxx LINK_LIBRARIES appfwk )

# ##############################################################################
# Test plugins
daq_add_plugin( DummyModule		  duneDAQModule TEST LINK_LIBRARIES appfwk )

# Test applications
daq_add_application( queue_IO_check queue_IO_check.cxx TEST LINK_LIBRARIES appfwk )
daq_add_application( dummy_module_test dummy_module_test.cxx TEST LINK_LIBRARIES appfwk )

# ##############################################################################
# Unit tests

daq_add_unit_test(Application_test            LINK_LIBRARIES appfwk )
daq_add_unit_test(CommandLineInterpreter_test LINK_LIBRARIES appfwk )
daq_add_unit_test(DAQModule_test              LINK_LIBRARIES appfwk )
daq_add_unit_test(DAQModuleManager_test       LINK_LIBRARIES appfwk )
daq_add_unit_test(DAQSink_DAQSource_test      LINK_LIBRARIES appfwk )
daq_add_unit_test(FollyQueue_test             LINK_LIBRARIES appfwk )
daq_add_unit_test(FollyQueue_metric_test      LINK_LIBRARIES appfwk )
daq_add_unit_test(Interruptible_test          LINK_LIBRARIES appfwk)
daq_add_unit_test(Queue_test                  LINK_LIBRARIES appfwk )
daq_add_unit_test(QueueRegistry_test          LINK_LIBRARIES appfwk )
daq_add_unit_test(StdDeQueue_test             LINK_LIBRARIES appfwk )
#daq_add_unit_test(ThreadHelper_test           LINK_LIBRARIES ers::ers)
daq_add_unit_test(NamedObject_test        )

##############################################################################

daq_install()
