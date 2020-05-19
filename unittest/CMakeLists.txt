
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_NO_BOOST_CMAKE ON)
set(BUILD_SHARED_LIBS ON)
find_package(Boost COMPONENTS unit_test_framework)

if(Boost_FOUND)
    add_subdirectory(DAQModules)
endif()
