set(SPACK_ROOT $ENV{SPACK_ROOT})
set(PATH_DIRS ${CMAKE_CURRENT_BINARY_DIR}:${CMAKE_CURRENT_BINARY_DIR}/../apps)
set(LIB_DIRS ${CMAKE_CURRENT_BINARY_DIR}/../src)
configure_file(setupForRunning.sh.in setupForRunning.sh @ONLY)