# Need cmake 3.14 so that FindLibLZMA creates the LibLZMA::LibLZMA target
cmake_minimum_required(VERSION 3.14)
project(appfwk VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_SHARED_LIBS on)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
include(DAQ)

find_package(cetlib MODULE REQUIRED) # MODULE because we use our own Findcetlib.cmake
find_package(folly REQUIRED)
find_package(ers REQUIRED)
find_package(Boost 1.70.0 COMPONENTS unit_test_framework program_options REQUIRED)
find_package(TRACE 3.15.09 REQUIRED)

##############################################################################
point_build_to( apps )

add_executable(daq_application apps/daq_application.cxx)
target_link_libraries(daq_application appfwk)

##############################################################################
point_build_to( doc )
# (No action taken, doc/CMakeLists.txt was empty)

##############################################################################
point_build_to( include )  
#set(PATH_DIRS ${CMAKE_CURRENT_BINARY_DIR}/scripts:${CMAKE_CURRENT_BINARY_DIR}/apps:${CMAKE_CURRENT_BINARY_DIR}/test)
#set(LIB_DIRS ${CMAKE_CURRENT_BINARY_DIR}/src:${CMAKE_CURRENT_BINARY_DIR}/test)
#configure_file(scripts/setupForRunning.sh.in scripts/setupForRunning.sh @ONLY)


##############################################################################
point_build_to( src )

add_library(appfwk SHARED src/QueueRegistry.cpp src/DAQProcess.cpp  src/DAQModule.cpp)
target_link_libraries(appfwk PUBLIC Folly::folly ers::ers cetlib::cetlib)
target_include_directories(appfwk
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
message(STATUS "CMAKE_CURRENT_SOURCE_DIR/include is ${CMAKE_CURRENT_SOURCE_DIR}/include")

add_library(appfwk_QueryResponseCommandFacility_duneCommandFacility src/QueryResponseCommandFacility.cpp)
target_link_libraries(appfwk_QueryResponseCommandFacility_duneCommandFacility appfwk)

macro(MakeDataTypeLibraries)
   set(oneValueArgs CPPTYPE PREFIX)
   cmake_parse_arguments(MAKE_DATATYPE "" "${oneValueArgs}" "" ${ARGN})

   configure_file(src/FanOutDAQModule.cpp.in src/${MAKE_DATATYPE_PREFIX}FanOutDAQModule.cpp)
   set(FANOUT_LIBNAME ${MAKE_DATATYPE_PREFIX}FanOutDAQModule_duneDAQModule)
   add_library(${FANOUT_LIBNAME} SHARED src/${MAKE_DATATYPE_PREFIX}FanOutDAQModule.cpp)
   target_link_libraries(${FANOUT_LIBNAME} appfwk)
endmacro()

MakeDataTypeLibraries(CPPTYPE std::vector<int> PREFIX VectorInt)
MakeDataTypeLibraries(CPPTYPE std::string PREFIX String)
MakeDataTypeLibraries(CPPTYPE dunedaq::appfwk::NonCopyableType PREFIX NonCopyableType)

##############################################################################
point_build_to( test )

add_executable(queue_IO_check test/queue_IO_check.cxx)
target_link_libraries(queue_IO_check appfwk)

add_library(appfwk_FakeDataConsumerDAQModule_duneDAQModule test/FakeDataConsumerDAQModule.cpp)
target_link_libraries(appfwk_FakeDataConsumerDAQModule_duneDAQModule appfwk)
add_library(appfwk_DummyModule_duneDAQModule test/DummyModule.cpp)
target_link_libraries(appfwk_DummyModule_duneDAQModule appfwk)

add_library(appfwk_FakeDataProducerDAQModule_duneDAQModule test/FakeDataProducerDAQModule.cpp)
target_link_libraries(appfwk_FakeDataProducerDAQModule_duneDAQModule appfwk)

add_executable(dummy_test_app test/dummy_test_app.cxx)
target_link_libraries(dummy_test_app appfwk appfwk_DummyModule_duneDAQModule)

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
    DIRECTORY include/nlohmann
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

install(FILES cmake/DAQ.cmake cmake/Findcetlib.cmake
  DESTINATION "${config_install_dir}/Modules"
)
