cmake_minimum_required(VERSION 3.12)
project(appfwk VERSION 6.2.0)

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
find_package(okssystem REQUIRED)
find_package(conffwk REQUIRED)
find_package(confmodel REQUIRED)
find_package(appmodel REQUIRED)


set(APPFWK_DEPENDENCIES ${CETLIB} ${CETLIB_EXCEPT} ers::ers logging::logging cmdlib::cmdlib rcif::rcif iomanager::iomanager opmonlib::opmonlib nlohmann_json::nlohmann_json 
  confmodel::confmodel appmodel::appmodel conffwk::conffwk okssystem::okssystem)

find_package(oksdalgen REQUIRED)
daq_add_dal_library(appfwk.schema.xml TEST NAMESPACE dunedaq::appfwk::dal
  DALDIR dal DEP_PKGS confmodel)

daq_protobuf_codegen( opmon/*.proto )

##############################################################################
# Main library
daq_add_library(Application.cpp DAQModule.cpp DAQModuleManager.cpp ConfigurationManager.cpp
  LINK_LIBRARIES ${APPFWK_DEPENDENCIES})

# ##############################################################################
# Applications
daq_add_application( daq_application daq_application.cxx LINK_LIBRARIES appfwk )

# ##############################################################################
# Test plugins
daq_add_plugin( DummyModule		  duneDAQModule TEST LINK_LIBRARIES appfwk )
daq_add_plugin( ExtraModule		  duneDAQModule TEST LINK_LIBRARIES appfwk )

# Test applications
daq_add_application( dummy_module_test dummy_module_test.cxx TEST LINK_LIBRARIES appfwk )
daq_add_application( check_action_plans check_action_plans.cxx TEST LINK_LIBRARIES appfwk )

# ##############################################################################
# Unit tests

daq_add_unit_test(Application_test            LINK_LIBRARIES appfwk )
daq_add_unit_test(CommandLineInterpreter_test LINK_LIBRARIES appfwk )
daq_add_unit_test(DAQModule_test              LINK_LIBRARIES appfwk )
daq_add_unit_test(DAQModuleManager_test       LINK_LIBRARIES appfwk )

##############################################################################

daq_install()
