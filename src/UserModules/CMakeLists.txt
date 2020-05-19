
file(GLOB source_files *.cc)
add_library(app-framework_UserModules SHARED ${source_files} )
