cmake_minimum_required(VERSION 3.12)
project(appfwk VERSION 1.0)

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

add_library(appfwk SHARED src/QueueRegistry.cpp src/DAQProcess.cpp)

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
target_link_libraries(queue_IO_check ${DAQ_LIBRARIES_UNIVERSAL_EXE}  Folly::folly)


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
add_unit_test(FanOutDAQModule_test)

##############################################################################
# Installation stuff copied from the double-conversion CMakeLists.txt, which in turn got it from
# https://github.com/forexample/package-example

include(GNUInstallDirs)

# Layout. This works for all platforms:
#   * <prefix>/lib/cmake/<PROJECT-NAME>
#   * <prefix>/lib/
#   * <prefix>/include/
set(config_install_dir "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")

set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

# Configuration
set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(targets_export_name "${PROJECT_NAME}Targets")
set(namespace "${PROJECT_NAME}::")

# Include module with function 'write_basic_package_version_file'
include(CMakePackageConfigHelpers)

# Configure '<PROJECT-NAME>ConfigVersion.cmake'
# Note: PROJECT_VERSION is used as a VERSION
write_basic_package_version_file(
    "${version_config}" COMPATIBILITY SameMajorVersion
)

# Configure '<PROJECT-NAME>Config.cmake'
# Use variables:
#   * targets_export_name
#   * PROJECT_NAME
configure_package_config_file(
    "cmake/Config.cmake.in"
    "${project_config}"
    INSTALL_DESTINATION "${config_install_dir}"
)

# Maybe also install the test apps in ${CMAKE_INSTALL_BINDIR}?
install(
    TARGETS appfwk daq_application appfwk_QueryResponseCommandFacility_duneCommandFacility
    EXPORT "${targets_export_name}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
    DIRECTORY include/appfwk
    DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

install(
    FILES "${project_config}" "${version_config}"
    DESTINATION "${config_install_dir}"
)

install(
    EXPORT "${targets_export_name}"
    NAMESPACE "${namespace}"
    DESTINATION "${config_install_dir}"
)
