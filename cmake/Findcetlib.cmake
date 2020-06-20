# cetlib has a cetlibConfig.cmake file, but that file references
# cetbuildtools modules, which cmake is having trouble finding. We
# probably would need to go the whole hog and make a
# cetbuildtools-style package. Plus, cetlib doesn't export any targets
# nicely, so we steal this bit of code from daq-buildtools and do it
# ourselves
if(EXISTS $ENV{CETLIB_LIB})
 # UPS
  find_library(CETLIB NAMES libcetlib.so)
  find_library(CETLIB_EXCEPT NAMES libcetlib_except.so)
else()
  # Spack
  find_package(cetlib REQUIRED)
  set(CETLIB cetlib)
  set(CETLIB_EXCEPT cetlib_except)
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(cetlib
  FOUND_VAR cetlib_FOUND
  REQUIRED_VARS
    CETLIB
    CETLIB_EXCEPT
)

if(cetlib_FOUND AND NOT TARGET cetlib_except::cetlib_except)
  # Create imported target 
  add_library(cetlib_except::cetlib_except SHARED IMPORTED)
  
  set_target_properties(cetlib_except::cetlib_except PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "$ENV{CETLIB_EXCEPT_INC}"
    # TODO: maybe we need to link against cetlib_except's dependencies too?
    # INTERFACE_LINK_LIBRARIES "$"
    IMPORTED_LOCATION "${CETLIB_EXCEPT}"
    )
endif()

if(cetlib_FOUND AND NOT TARGET cetlib::cetlib)
  # Create imported target 
  add_library(cetlib::cetlib SHARED IMPORTED)
  
  set_target_properties(cetlib::cetlib PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "$ENV{CETLIB_INC}"
    INTERFACE_LINK_LIBRARIES "cetlib_except::cetlib_except"
    IMPORTED_LOCATION "${CETLIB}"
    )
  
endif()
