cmake_minimum_required(VERSION 3.12)
project(appfwk VERSION 3.0.1)

find_package(daq-cmake REQUIRED )

daq_setup_environment()

find_package(Boost COMPONENTS unit_test_framework program_options REQUIRED)
find_package(logging REQUIRED)
find_package(cetlib REQUIRED)   # Uses the daq-buildtools/cmake/Findcetlib.cmake
find_package(ers REQUIRED)
find_package(cmdlib REQUIRED)
find_package(rcif REQUIRED)
find_package(iomanager REQUIRED)
find_package(opmonlib REQUIRED)
find_package(nlohmann_json REQUIRED )
find_package(Pistache REQUIRED)
find_package(okssystem REQUIRED)
find_package(conffwk REQUIRED)
find_package(confmodel REQUIRED)
find_package(appmodel REQUIRED)


set(APPFWK_DEPENDENCIES ${CETLIB} ${CETLIB_EXCEPT} ers::ers logging::logging cmdlib::cmdlib rcif::rcif iomanager::iomanager opmonlib::opmonlib nlohmann_json::nlohmann_json pistache_shared
  confmodel::confmodel appmodel::appmodel conffwk::conffwk okssystem::okssystem)

find_package(oksdalgen REQUIRED)
daq_oks_codegen(appfwk.schema.xml TEST NAMESPACE dunedaq::appfwk::dal
  DALDIR dal DEP_PKGS confmodel)

daq_codegen( app.jsonnet cmd.jsonnet DEP_PKGS iomanager rcif cmdlib TEMPLATES Structs.hpp.j2 Nljs.hpp.j2 )
daq_codegen( appinfo.jsonnet DEP_PKGS opmonlib TEMPLATES opmonlib/InfoStructs.hpp.j2 opmonlib/InfoNljs.hpp.j2 )

##############################################################################
# Main library
daq_add_library(DAQModule*.cpp ConfigurationManager.cpp ModuleConfiguration.cpp
  LINK_LIBRARIES ${APPFWK_DEPENDENCIES})

##############################################################################
# Plugins
daq_add_plugin(fileConfFacility duneConfFacility LINK_LIBRARIES ers::ers logging::logging nlohmann_json::nlohmann_json)
daq_add_plugin(dbConfFacility duneConfFacility LINK_LIBRARIES ers::ers logging::logging nlohmann_json::nlohmann_json pistache_shared)

# ##############################################################################
# Applications
daq_add_application( daq_application daq_application.cxx LINK_LIBRARIES appfwk )

# ##############################################################################
# Test plugins
daq_add_plugin( DummyModule		  duneDAQModule TEST LINK_LIBRARIES appfwk )

# Test applications
daq_add_application( dummy_module_test dummy_module_test.cxx TEST LINK_LIBRARIES appfwk )

# ##############################################################################
# Unit tests

daq_add_unit_test(Application_test            LINK_LIBRARIES appfwk )
daq_add_unit_test(CommandLineInterpreter_test LINK_LIBRARIES appfwk )
daq_add_unit_test(DAQModuleManager_test       LINK_LIBRARIES appfwk )
daq_add_unit_test(Interruptible_test          LINK_LIBRARIES appfwk )

##############################################################################

daq_install()
