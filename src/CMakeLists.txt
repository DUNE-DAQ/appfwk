
file(GLOB source_files *.cc)
add_library(app-framework SHARED ${source_files} )

add_subdirectory(UserModules)

