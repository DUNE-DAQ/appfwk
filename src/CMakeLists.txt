
file(GLOB source_files *.cc)

list(FILTER source_files EXCLUDE REGEX "QueryResponseCommandFacility.cc")
add_library(app-framework SHARED ${source_files})
target_link_libraries(app-framework ${CETLIB} ${CETLIB_EXCEPT})

add_subdirectory(DAQModules)

add_library(app-framework_QueryResponseCommandFacility_duneCommandFacility QueryResponseCommandFacility.cc)

macro(MakeDataTypeLibraries)
   set(oneValueArgs CPPTYPE PREFIX)
   cmake_parse_arguments(MAKE_DATATYPE "" "${oneValueArgs}" "" ${ARGN})

   configure_file(StdDeQueue.cc.in ${MAKE_DATATYPE_PREFIX}StdDeQueue.cc)
   add_library(${MAKE_DATATYPE_PREFIX}StdDeQueue_duneObject SHARED ${MAKE_DATATYPE_PREFIX}StdDeQueue.cc)

   configure_file(FanOutDAQModule.cc.in ${MAKE_DATATYPE_PREFIX}FanOutDAQModule.cc)
   add_library(${MAKE_DATATYPE_PREFIX}FanOutDAQModule_duneObject SHARED ${MAKE_DATATYPE_PREFIX}FanOutDAQModule.cc)

endmacro()

MakeDataTypeLibraries(CPPTYPE std::vector<int> PREFIX VectorInt)
MakeDataTypeLibraries(CPPTYPE std::string PREFIX String)
MakeDataTypeLibraries(CPPTYPE appframework::NonCopyableType PREFIX NonCopyableType)
