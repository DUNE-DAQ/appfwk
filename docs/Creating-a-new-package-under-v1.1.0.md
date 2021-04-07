
## Setting up a development area

To create a new package, you'll want to install a DUNE-DAQ development environment and then create a new CMake project for the package. How to install and build the DUNE-DAQ development environment is described [[here|Compiling-and-running-under-v1.1.0]].

By the time you've set up the development environment as described in the linked instructions, you'll have already downloaded, built, and locally installed the CMake project for appfwk. You'll likely also have noticed that `./appfwk` is a git repo; you'll want your project to reside in a git repo as well. 

## A package's subdirectory structure

To learn a bit more about how to structure your package so that it can be incorporated into the DUNE DAQ software suite, we'll play with a contrived package called "toylibrary". It's actually contained within a subdirectory of the daq-buildtools repo; however, in order to be able to build toylibrary we'll want to copy it into the base directory of the development area. Assuming you're already in the base directory, just do 
```
cp -rp daq-buildtools/toylibrary .
```
You can now build it using the normal commands:
```
. ./setup_build_environment
./build_daq_software.sh --install --pkgname toylibrary
```
In terms of its actual functionality, it's pretty useless (it contains a class which can wrap an integer, and another class which can print that wrapped integer). However, its functionality is beside the point; toylibrary contains many features which DUNE DAQ packages have in common, in particular DUNE DAQ packages which provide a libraries other developers want to link against. In fact, if you run `ls appfwk` and `ls toylibrary`, you'll notice many subdirectories in common. These include:

* *src*: contains the source files meant to be built into the package's shared object library/libraries
* *include*: contains the headers users of your package should #include
* *unittest*: contains the [unit tests](https://en.wikipedia.org/wiki/Unit_testing) you write to ensure that your individual classes, functions, etc. behave as expected
* *test*: contains any applications you've written for the purpose of integration testing - ensuring that your software components interact as expected

If your package contains applications intended not for testing but for the end user, you'd put the code for it in a subdirectory called *apps*. toylibrary doesn't have this type of application, but the appfwk package does. 

## Coding rules

Along with having a standard directory structure, the C++ code itself in toylibrary conforms to the [DUNE C++ Style Guide](https://github.com/DUNE-DAQ/styleguide/blob/develop/dune-daq-cppguide.md). Here, "style" doesn't mean whitespace and formatting, but rather, a set of Modern C++ best practices designed to make your code more robust against bugs, easier to extend, easier to reuse, etc. The DUNE C++ Style Guide is derived from the Google C++ Style Guide, but is greatly simplified and has been modified to be more appropriate to the DUNE DAQ project than Google's projects. Code which is merged into a package's git develop branch should be in conformance with the guide; while it's encouraged for code on a package's unmerged feature branches to also be in conformance, this is less important. 

## Your project's CMakeLists.txt file

Every DUNE DAQ package should have one and only one CMakeLists.txt file, in the base directory of the package (not to be confused with the base directory of the overall development area). To learn a bit about what that CMakeLists.txt file should look like, let's take a look at `./toylibrary/CMakeLists.txt`. Because CMake is widely used and extensively documented online, this documentation will primarily focus on DUNE-specific CMake functions. 

Before doing anything else, we want to define the minimum version of CMake used (currently 3.12, which supports [modern CMake style](https://cliutils.gitlab.io/modern-cmake/)) as well as the name and version of the project. Concerning the version: it may not literally be the case that the code you're working with is exactly the same as the version-in-question's release code, because you may be on a feature branch, or there may have been commits to the develop branch since the last release. 
```
cmake_minimum_required(VERSION 3.12)
project(toylibrary VERSION 1.1.0)
```
Next, we want to make CMake functions written specifically for DUNE DAQ development available. Near the top of the file, you'll see the following:
```
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../daq-buildtools/cmake ${CMAKE_MODULE_PATH})
include(DAQ)
```
This is currently (Aug-5-2020) how we ensure that the CMakeLists.txt file has access to the standard DUNE DAQ CMake functions. It imports the `DAQ` CMake module which is located in the daq-buildtools repo that's downloaded when you ran quick-start.sh. Note that by convention all functions/macros within the module begin with `daq_`, so as to distinguish them from functions/macros from CMake modules written outside of DUNE DAQ. 

The next step is to call a macro from the `DAQ` module which sets up a standard DUNE CMake environment for your CMakeLists.txt file:
```
daq_setup_environment()
```
To get specific, daq_setup_environment() will do the following:
* Enforce the use of standard, extension-free C++17
* Ensure libraries are built as shared, rather than static
* Ensure all code within the project can find the project's public headers
* Allow our linter scripts to work with the code 
* Tell the CMake `find_package` function to look in the local `./install` subdirectory of the development area for packages (e.g., appfwk)
* Have gcc use standard warnings
* Support the use of CTest for the unit tests

Next you'll see calls to CMake's [find_package](https://cmake.org/cmake/help/v3.17/command/find_package.html) function, which makes toylibrary's dependencies available. Comments in the file explain why the dependencies are selected. 

Then, you'll see a call to a function called `daq_point_build_to`. CMake commands which are specific to the code in a particular subdirectory tree of your project should be clustered in the same location of the the CMakeLists.txt file, and they should be prefaced with the name of the subdirectory wrapped in the `daq_point_build_to` CMake function. E.g., for the code in `./toylibrary/src` we have:
```
daq_point_build_to( src )

add_library(               toylibrary       src/IntPrinter.cpp )
```
Here, `daq_point_build_to(src)` will ensure that the ensuing target(s) -- here, the toylibrary shared object file -- will be placed in a subdirectory of `./build/toylibrary` directory (build directory) whose path mirrors the path of the source itself. To get concrete, this command means that after a successful build we'll have an executable, `./build/toylibrary/src/libtoylibrary.so`, where that path is given relative to the base of your entire development area. Without this function call it would end up as `./build/toylibrary/libtoylibrary.so`. With a small project such as toylibrary the benefit may not be obvious, but it becomes more so when many libraries, applications, unit tests, etc. are built within a project.  

Another function currently provided by the DAQ CMake module is `daq_add_unit_test`. Examples of this function's use can be found at the bottom of the `toylibrary/CMakeLists.txt` file, e.g.:
```
daq_add_unit_test(ValueWrapper_test)
```
If you pass this function a name, e.g., `MyComponent_test`, it will create a unit test executable off of a source file called `<your packagename>/unittest/MyComponent_test.cxx`, and handle linking in the Boost unit test dependencies. You can also optionally have it link in other libraries by providing them as arguments; in the above example, this isn't needed because ValueWrapper is a template class which is instantiated within the unit test code itself. You can see this linking, however, in `./appfwk/CMakeLists.txt`.  

At the bottom of CMakeLists.txt, you'll see the following function:
```
daq_install(TARGETS toylibrary)
```
The signature of this function is hopefully self-explanatory; basically when you call it it will install the targets (executables, shared object libraries) you wish to make available to others who want to use your package in a directory called `./install/<pkgname>` (here, of course, that would be `./install/toylibrary`). You'll also need to add a special file to your project for this function to work; this is discussed more fully in the "Installing your project as a local package" section later in this document. 

## Some general comments about CMake

While a lot can be learned from looking at the CMakeLists.txt file and reading this documentation, realize that CMake is a full-blown programming language, and like any programming language, the more familiar you are with it the easier life will be. It's extensively documented online; in particular, if you want to learn more about specific CMake functions you can go [here](https://cmake.org/cmake/help/v3.17/manual/cmake-commands.7.html) for a reference guide of CMake commands, or, if you've sourced `setup_build_environment` (so CMake is set up), you can even learn at the command line via `cmake --help-command <name of command>`

When you write code in CMake good software development principles generally apply. Writing your own macros and functions you can live up to the [Don't Repeat Yourself principle](https://en.wikipedia.org/wiki/Don%27t_repeat_yourself); an example of this can be found in the definition of the `MakeDataTypeLibraries` macro in `./appfwk/CMakeLists.txt`, which helps avoid boilerplate while instantiating appfwk's FanOutDAQModule template class for various types and giving these instantiations well-motivated names. 

Another thing to be aware of: since you're using a single CMakeLists.txt file, the scope of your decisions can extend to all ensuing code. So, e.g., if you add the line `link_libraries( bloated_library_with_extremely_common_symbol_names )` then all ensuing targets from `add_executable`, `add_library`, etc., will get that linked in. For this reason, prefer a target-specific call such as `target_link_libraries(executable_that_really_needs_this_library bloated_library_with_extremely_common_symbol_names)` instead. In modern CMake it's considered a best practice to avoid functions which globally link in libraries or include directories, and to make things target-specific instead. 

## If your package relies on nonstandard dependencies

As you've discovered from the Compiling-and-Running section you were pointed to at the start of this Wiki, `quick-start.sh` constructs a file called `setup_build_environment` which does what it says it's going to do. It performs ups setups of dependencies (Boost, etc.) which `appfwk` specifically, and in general most likely any package you'll write, rely on. If you have any new dependencies which are stored as ups products in the /cvmfs/dune.opensciencegrid.org/dunedaq/DUNE/products/ ups products area, you'll want to add them by looking for the `setup_returns` variable in `setup_build_environment`. You'll see there are several packages where for each package, there's a ups setup followed by an appending of the return value of the setup to `setup_returns`. You should do the same for each dependency you want to add, e.g.

```
setup <dependency_I_am_introducing> <version_of_the_dependency> # And perhaps qualifiers, e.g., -q e19:debug
setup_returns=$setup_returns"$? "
```

The `setup_build_environment` script uses `setup_returns` to check that all the packages were setup without a nonzero return value. 

You may also want to set up additional environment variables for your new project in `setup_build_environment`. 

## Installing your project as a local package

Use the procedure described below in order to have your package installed. Once your package is installed, it means other packages can access the libraries, public headers, etc., provided by your package via CMake's `find_package` command, i.e.:
```
# If other users call this, they can use your code
find_package(mypackage)
```

For starters, you'll want to call the DAQ module's `daq_install` function at the bottom of your CMakeLists.txt file. How to do so has already been described earlier in this document. 

You'll also want to make sure that projects which link in your code know where to find your headers. If you look in toylibrary's CMakeLists.txt file, you'll see the following:
```
target_include_directories(toylibrary
                           PUBLIC
                           $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                           )
```
...and what this will do is tell targets in other projects where to find headers when they link in the appfwk library. The code uses [CMake generator expressions](https://cmake.org/cmake/help/latest/manual/cmake-generator-expressions.7.html) so that the target will know either to look in a subdirectory of your `./install` area or a local build directory depending on whether or not it's using an installed version of appfwk. ${CMAKE_INSTALL_INCLUDEDIR} is set in the GNUInstallDirs module, which is imported as a consequence of importing our DAQ module. The value of the variable defaults to "include" and it's unlikely you'd want to change it.  

A major thing you should be aware of is that when you call CMake's `find_package` function, it will look for a file with the name `mypackageConfig.cmake` in a predetermined set of directories. The `daq_setup_environment` function discussed at the top of this page ensures that one of those directories is your local `./install/mypackage` directory. What a standard `mypackageConfig.cmake` file should look like with modern CMake is documented in many places on the web, but in order to make life as easy as possible there's a templatized version of this file in the daq-buildtools package. Assuming you've got a `./mypackage` repo in your development area, you can do the following:
```
cd ./mypackage
cp ../daq-buildtools/configs/Config.cmake.in mypackageConfig.cmake.in
```
and then let's look the contents of `mypackageConfig.cmake.in`:
```

@PACKAGE_INIT@

include(CMakeFindDependencyMacro)

# Insert find_dependency() calls for your package's dependencies in                                                        
# the place of this comment. Make sure they match up with the                                                              
# find_package calls in your package's CMakeLists.txt file                                                                 

set_and_check(targets_file ${CMAKE_CURRENT_LIST_DIR}/@PROJECT_NAME@Targets.cmake)
include(${targets_file})

check_required_components(@PROJECT_NAME@)
```
The only part of this file you need to worry about is the comment. In place of this comment, you'll want to call CMake's `find_dependency` function (details [here](https://cmake.org/cmake/help/latest/module/CMakeFindDependencyMacro.html)) for each package that mypackage depends on; this ensures that developers who call `find_package(mypackage)` don't need to have explicit `find_package` calls on these dependencies.

You can see a simple example of this kind of file with `./toylibrary/toylibraryConfig.cmake.in`, and a slightly less simple example with `./appfwk/appfwkConfig.cmake.in`.

Once you've edited this file as described, from the base of your development area you can then run 
```
./build_daq_software.sh --install --pkgname mypackage
```
without receiving an error message informing you that installation isn't an option. 
