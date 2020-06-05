cmake_minimum_required(VERSION 3.12)
project(app-framework)

set(CMAKE_CXX_STANDARD 17)

find_package(TRACE 3.15.0 REQUIRED)

find_package(nlohmann_json 3.2.0)
if(NOT ${nlohmann_json_FOUND})
  message("nlohmann_json NOT FOUND! Downloading single-header from GitHub!")
  file(DOWNLOAD https://github.com/nlohmann/json/raw/develop/single_include/nlohmann/json.hpp nlohmann/json.hpp)
  include_directories(${CMAKE_BINARY_DIR})
endif()


set(Boost_USE_STATIC_LIBS OFF)
set(Boost_NO_BOOST_CMAKE ON)
set(BUILD_SHARED_LIBS ON)
find_package(Boost 1.70.0 COMPONENTS unit_test_framework program_options REQUIRED)


if(EXISTS $ENV{CETLIB_LIB})
# UPS
  include_directories($ENV{CETLIB_INC})
  include_directories($ENV{CETLIB_EXCEPT_INC})
  find_library(CETLIB NAMES libcetlib.so)
  find_library(CETLIB_EXCEPT NAMES libcetlib_except.so)
else()
	# Spack
	find_package(cetlib REQUIRED)
	set(CETLIB cetlib)
	set(CETLIB_EXCEPT cetlib_except)
endif()

include_directories(${CMAKE_SOURCE_DIR}/app-framework/include)
include_directories(${CMAKE_SOURCE_DIR}/app-framework-base/include)
include_directories(${CMAKE_SOURCE_DIR}/ers)

function( point_build_to output_dir )

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${output_dir} PARENT_SCOPE)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${output_dir} PARENT_SCOPE)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${output_dir} PARENT_SCOPE)

endfunction()

point_build_to( apps )

include_directories(${CMAKE_SOURCE_DIR}/app-framework/include)
include_directories(${CMAKE_SOURCE_DIR}/app-framework-base/include)

add_executable(daq_application apps/daq_application.cc)
target_link_libraries(daq_application app-framework  pthread ers ${CETLIB} ${CETLIB_EXCEPT} ${Boost_PROGRAM_OPTIONS_LIBRARY})

point_build_to( doc )
# (No action taken, doc/CMakeLists.txt was empty)

point_build_to( include )  

install(DIRECTORY include DESTINATION include)


set(SPACK_ROOT $ENV{SPACK_ROOT})
set(PATH_DIRS ${CMAKE_CURRENT_BINARY_DIR}/scripts:${CMAKE_CURRENT_BINARY_DIR}/apps)
set(LIB_DIRS ${CMAKE_CURRENT_BINARY_DIR}/src)
configure_file(scripts/setupForRunning.sh.in scripts/setupForRunning.sh @ONLY)



point_build_to( src )
file(GLOB source_files src/*.cc)

list(FILTER source_files EXCLUDE REGEX "QueryResponseCommandFacility.cc")
add_library(app-framework SHARED ${source_files})
target_link_libraries(app-framework ${CETLIB} ${CETLIB_EXCEPT})


add_library(app-framework_QueryResponseCommandFacility_duneCommandFacility src/QueryResponseCommandFacility.cc)

macro(MakeDataTypeLibraries)
   set(oneValueArgs CPPTYPE PREFIX)
   cmake_parse_arguments(MAKE_DATATYPE "" "${oneValueArgs}" "" ${ARGN})

   configure_file(src/FanOutDAQModule.cc.in src/${MAKE_DATATYPE_PREFIX}FanOutDAQModule.cc)
   add_library(${MAKE_DATATYPE_PREFIX}FanOutDAQModule_duneDAQModule SHARED src/${MAKE_DATATYPE_PREFIX}FanOutDAQModule.cc)

endmacro()

MakeDataTypeLibraries(CPPTYPE std::vector<int> PREFIX VectorInt)
MakeDataTypeLibraries(CPPTYPE std::string PREFIX String)
MakeDataTypeLibraries(CPPTYPE appframework::NonCopyableType PREFIX NonCopyableType)


point_build_to( test )

set(Boost_NO_BOOST_CMAKE ON)
find_package(Boost COMPONENTS program_options REQUIRED)

add_executable(queue_IO_check test/queue_IO_check.cc)
target_link_libraries(queue_IO_check pthread ${Boost_PROGRAM_OPTIONS_LIBRARY} )

add_library(app-framework_DebugLoggingDAQModule_duneDAQModule test/DebugLoggingDAQModule.cc)
add_library(app-framework_FakeDataConsumerDAQModule_duneDAQModule test/FakeDataConsumerDAQModule.cc)
add_library(app-framework_FakeDataProducerDAQModule_duneDAQModule test/FakeDataProducerDAQModule.cc)

add_executable(echo_test_app test/echo_test_app.cc)
target_link_libraries(echo_test_app app-framework app-framework_DebugLoggingDAQModule_duneDAQModule pthread ers  ${CETLIB} ${CETLIB_EXCEPT} ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable(producer_consumer_test_app test/producer_consumer_test_app.cc)
target_link_libraries(producer_consumer_test_app app-framework app-framework_FakeDataProducerDAQModule_duneDAQModule ers app-framework_FakeDataConsumerDAQModule_duneDAQModule pthread  ${CETLIB} ${CETLIB_EXCEPT} ${Boost_PROGRAM_OPTIONS_LIBRARY})

add_executable(daq_sink_source test/daq_sink_source.cc)
target_link_libraries(daq_sink_source app-framework pthread ers)


point_build_to( unittest )

if(Boost_FOUND)

include_directories(${Boost_INCLUDE_DIRS})

    add_executable(StdDeQueue_test unittest/StdDeQueue_test.cc)
    add_test(NAME StdDeQueue_test COMMAND StdDeQueue_test)

        add_executable(FanOutDAQModule_test unittest/FanOutDAQModule_test.cc)
    target_link_libraries(FanOutDAQModule_test app-framework ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} pthread ers)
target_compile_definitions(FanOutDAQModule_test PRIVATE "BOOST_TEST_DYN_LINK=1")
    target_include_directories(FanOutDAQModule_test PRIVATE ${Boost_INCLUDE_DIRS})
    add_test(NAME FanOutDAQModule_test COMMAND FanOutDAQModule_test ers)

add_executable(DAQModuleThreadHelper_test unittest/DAQModuleThreadHelper_test.cc)
target_link_libraries(DAQModuleThreadHelper_test ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY} pthread)
target_compile_definitions(DAQModuleThreadHelper_test PRIVATE "BOOST_TEST_DYN_LINK=1")
target_include_directories(DAQModuleThreadHelper_test PRIVATE ${Boost_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/app-framework-base/include)
add_test(NAME DAQModuleThreadHelper_test COMMAND DAQModuleThreadHelper_test)

endif()




