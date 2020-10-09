cmake_minimum_required(VERSION 3.12)
project(appfwk VERSION 1.1.0)

find_package(daq-buildtools REQUIRED )

daq_setup_environment()

find_package(Boost 1.70.0 COMPONENTS unit_test_framework program_options REQUIRED)
find_package(TRACE 3.15.09 REQUIRED)
find_package(cetlib REQUIRED)   # Uses the daq-buildtools/cmake/Findcetlib.cmake
find_package(folly REQUIRED)
find_package(ers REQUIRED)
find_package(nlohmann_json REQUIRED )

set(APPFWK_DEPENDENCIES ${CETLIB} ${CETLIB_EXCEPT} ers::ers Folly::folly nlohmann_json::nlohmann_json pthread)

##############################################################################
# Main library
daq_add_library(*.* LINK_LIBRARIES ${CETLIB} ${CETLIB_EXCEPT} ers::ers Folly::folly nlohmann_json::nlohmann_json pthread)

# add_library(appfwk_QueryResponseCommandFacility_duneCommandFacility src/QueryResponseCommandFacility.cpp)
# target_link_libraries(appfwk_QueryResponseCommandFacility_duneCommandFacility appfwk)

# macro(MakeDataTypeLibraries)
#    set(oneValueArgs CPPTYPE PREFIX)
#    cmake_parse_arguments(MAKE_DATATYPE "" "${oneValueArgs}" "" ${ARGN})

#    configure_file(src/FanOutDAQModule.cpp.in src/${MAKE_DATATYPE_PREFIX}FanOutDAQModule.cpp)
#    add_library(${MAKE_DATATYPE_PREFIX}FanOutDAQModule_duneDAQModule SHARED src/${MAKE_DATATYPE_PREFIX}FanOutDAQModule.cpp)
#    target_link_libraries(${MAKE_DATATYPE_PREFIX}FanOutDAQModule_duneDAQModule appfwk)

# endmacro()

# MakeDataTypeLibraries(CPPTYPE std::vector<int> PREFIX VectorInt)
# MakeDataTypeLibraries(CPPTYPE std::string PREFIX String)
# MakeDataTypeLibraries(CPPTYPE dunedaq::appfwk::NonCopyableType PREFIX NonCopyableType)


##############################################################################
# Plugins
daq_add_plugin(FileCommandFacility duneCommandFacility LINK_LIBRARIES appfwk)


# ##############################################################################
# Applications
daq_add_application( daq_application daq_application.cxx LINK_LIBRARIES appfwk )


# ##############################################################################
# # (No action taken, doc/ doesn't yet contain anything)

# ##############################################################################
# Test plugins
daq_add_plugin( DummyModule duneDAQModule TEST LINK_LIBRARIES appfwk )
daq_add_plugin( FakeDataConsumerDAQModule duneDAQModule TEST LINK_LIBRARIES appfwk )
daq_add_plugin( FakeDataProducerDAQModule duneDAQModule TEST LINK_LIBRARIES appfwk )

# Test applications
daq_add_application( queue_IO_check queue_IO_check.cxx TEST LINK_LIBRARIES appfwk )

file(COPY test/producer_consumer_dynamic_test.json DESTINATION test)

# ##############################################################################
# Unit tests

daq_add_unit_test(DAQModule_test          LINK_LIBRARIES appfwk )
daq_add_unit_test(DAQSink_DAQSource_test  LINK_LIBRARIES appfwk )
daq_add_unit_test(FanOutDAQModule_test    LINK_LIBRARIES appfwk )
daq_add_unit_test(FollyQueue_test         LINK_LIBRARIES ers::ers Folly::folly)
daq_add_unit_test(StdDeQueue_test         LINK_LIBRARIES ers::ers)
daq_add_unit_test(ThreadHelper_test       LINK_LIBRARIES ers::ers)
daq_add_unit_test(NamedObject_test        LINK_LIBRARIES appfwk )

##############################################################################

daq_install(TARGETS daq_application 
                    appfwk
		    # appfwk_QueryResponseCommandFacility_duneCommandFacility 
                    appfwk_fileCommandFacility_duneCommandFacility
                    VectorIntFanOutDAQModule_duneDAQModule
                    StringFanOutDAQModule_duneDAQModule
                    NonCopyableTypeFanOutDAQModule_duneDAQModule
	   )
