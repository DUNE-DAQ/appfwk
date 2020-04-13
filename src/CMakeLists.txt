
file(GLOB source_files *.cc)
list(FILTER source_files EXCLUDE REGEX "QueryResponseCommandFacility.cc")
add_library(app-framework ${source_files})

add_library(app-framework_QueryResponseCommandFacility_duneCommandFacility QueryResponseCommandFacility.cc)