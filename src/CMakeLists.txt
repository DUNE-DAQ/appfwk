
file(GLOB source_files *.cc)

list(FILTER source_files EXCLUDE REGEX "QueryResponseCommandFacility.cc")
add_library(app-framework SHARED ${source_files})
target_link_libraries(app-framework cetlib cetlib_except)

add_subdirectory(UserModules)

add_library(app-framework_QueryResponseCommandFacility_duneCommandFacility QueryResponseCommandFacility.cc)
