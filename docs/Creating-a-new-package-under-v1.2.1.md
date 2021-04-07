
## Setting up a development area

To create a new package, you'll want to install a DUNE-DAQ development environment and then create a new CMake project for the package. How to install and build the DUNE-DAQ development environment is described in [[Compiling and running under v1.2.1|Compiling-and-running-under-v1.2.1]].

By the time you've set up the development environment as described in the linked instructions, you'll have already downloaded, built, and locally installed the CMake project for appfwk. You'll have observed that `./appfwk` is a git repo; you'll want your project to reside in a git repo as well. 

## A package's subdirectory structure

To learn a bit more about how to structure your package so that it can be incorporated into the DUNE DAQ software suite, we'll play with a contrived package called "toylibrary". It's actually contained within a subdirectory of the daq-buildtools repo; however, in order to be able to build toylibrary we'll want to copy it into the `./sourcecode` directory so the build syste, can work with it. Assuming you're already in the base directory, just do 
```
cp -rp sourcecode/daq-buildtools/toylibrary sourcecode
```
Since toylibrary doesn't depend on any other repos, and no repos depend on toylibrary, we won't need to add its name to the `build_order` list in `sourcecode/CMakeLists.txt` to tell daq-buildtools what packages to build before toylibrary, and what packages to build after. However, adding it will get rid of a warning message which would otherwise appear. 

You can now build toylibrary using the normal commands:
```
. ./setup_build_environment 
./build_daq_software.sh --install --clean
```
In terms of its actual functionality, it's pretty useless (it contains a class which can wrap an integer, and another class which can print that wrapped integer). However, its functionality is beside the point; toylibrary contains many features which DUNE DAQ packages have in common, in particular DUNE DAQ packages which provide a library other developers want to link against. In fact, if you run `ls sourcecode/appfwk` and `ls sourcecode/toylibrary`, you'll notice many subdirectories in common. These include:

* *src*: contains the source files meant to be built into the package's shared object library/libraries
* *include*: contains the headers users of your package should #include
* *unittest*: contains the [unit tests](https://en.wikipedia.org/wiki/Unit_testing) you write to ensure that your individual classes, functions, etc. behave as expected
* *test*: contains any applications or plugins you've written for the purpose of [integration testing](https://en.wikipedia.org/wiki/Integration_testing) - ensuring that your software components interact as expected

If your package contains applications intended not for testing but for the end user, you'd put the code for it in a subdirectory called *apps*. toylibrary doesn't have this type of application, but the appfwk package does. Similarly, plugins not intended for testing but for the end user would go in *plugins*. 

## Coding rules

Along with having a standard directory structure, the C++ code itself in toylibrary conforms to the [DUNE C++ Style Guide](https://github.com/DUNE-DAQ/styleguide/blob/develop/dune-daq-cppguide.md). Here, "style" doesn't mean whitespace and formatting, but rather, a set of Modern C++ best practices designed to make your code more robust against bugs, easier to extend, easier to reuse, etc. The DUNE C++ Style Guide is derived from the Google C++ Style Guide, but is greatly simplified and has been modified to be more appropriate to the DUNE DAQ project than Google's projects. Code which is merged into a package's git develop branch should be in conformance with the guide; while it's encouraged for code on a package's unmerged feature branches to also be in conformance, this is less important. 

## Your project's CMakeLists.txt file

Every DUNE DAQ package should have one and only one CMakeLists.txt file, in the base directory of the package (not to be confused with the base directory of the overall development area). To learn a bit about what that CMakeLists.txt file should look like, let's take a look at `sourcecode/toylibrary/CMakeLists.txt`. Because CMake is widely used and extensively documented online, this documentation will primarily focus on DUNE-specific CMake functions. The full documentation of the DUNE-specific CMake functions for users can be found as comments in the body of the CMake module which contains them. You'll have daq-buildtools in your repo area so you can actually look at this module, `./sourcecode/daq-buildtools/cmake/DAQ.cmake`. Depending on your learning style, however, you may find it easier to start learning about some of what these functions are capable of by reading on in this wiki. 

At the top of CMakeLists.txt: before doing anything else, we want to define the minimum version of CMake used (currently 3.12, which supports [modern CMake style](https://cliutils.gitlab.io/modern-cmake/)) as well as the name and version of the project. Concerning the version: it may not literally be the case that the code you're working with is exactly the same as the version-in-question's release code, because you may be on a feature branch, or there may have been commits to the develop branch since the last release. 
```
cmake_minimum_required(VERSION 3.12)
project(toylibrary VERSION 1.2.0)
```
Next, we want to make CMake functions written specifically for DUNE DAQ development available. Near the top of the file, you'll see the following:
```
find_package(daq-buildtools REQUIRED)
```
This is how we ensure that the CMakeLists.txt file has access to the standard DUNE DAQ CMake functions previously mentioned. When `find_package` is called here it imports daq-buildtools' `DAQ` CMake module. Note that by convention all functions/macros within the module begin with `daq_`, so as to distinguish them from functions/macros from CMake modules written outside of DUNE DAQ. 

The next step is to call a macro from the `DAQ` module which sets up a standard DUNE CMake environment for your CMakeLists.txt file:
```
daq_setup_environment()
```
To get specific, daq_setup_environment() will do the following:
* Enforce the use of standard, extension-free C++17
* Ensure all code within the project can find the project's public headers
* Allow our linter scripts to work with the code 
* Have gcc use standard warnings
* Support the use of CTest for the unit tests

Next you'll see calls to CMake's [find_package](https://cmake.org/cmake/help/v3.17/command/find_package.html) function, which makes toylibrary's dependencies available. Comments in the file explain why the dependencies are selected. 

Then, you'll see a call to a function called `daq_add_library`. 
```
daq_add_library(IntPrinter.cpp LINK_LIBRARIES ers::ers)
```
What `daq_add_library` does here is create the main project library. It looks in the project's `./src` subdirectory for a file called `IntPrinter.cpp`, which it then compiles and links against the ERS library. The result is output in the build area as a shared object library named after the project itself, in a subdirectory of the same name as that of the source file it used - `build/toylibrary/src/libtoylibrary.so`. When `--install` is passed to `./build_daq_software.sh`, as it was in the instructions above, this library in turn is installed in a subdirectory of the installation area called `toylibrary/lib64/libtoylibrary.so`. 

The next function you see called in the CMakeLists.txt file is `daq_add_application`:
```
daq_add_application( toylibrary_test_program toylibrary_test_program.cxx TEST LINK_LIBRARIES ${Boost_PROGRAM_OPTIONS_LIBRARY} toylibrary )
```
which searches in the projects' `test/apps/` subdirectory for a file called `toylibrary_test_program.cxx`, builds it, and links against the project's main library which we created via the previous `daq_add_library` command as well as a Boost library used to parse program input. The output application is named after the first argument to the function, `toylibrary_test_program`; it can be found in `build/toylibrary/test/apps/toylibrary_test_program`. Since "TEST" was selected, the application won't be placed in the install area despite `--install` being used. Note that if the "TEST" argument hadn't been supplied, along with the installation occuring, the build system would have looked in a subdirectory of the project called `apps/` rather than `test/apps/` for the source file. In this sense, `toylibrary_test_program` is an example of an integration test program of interest to a project's developers but not its users. 

Another function currently provided by the DAQ CMake module is `daq_add_unit_test`. Examples of this function's use can be found at the bottom of the `sourcecode/toylibrary/CMakeLists.txt` file, e.g.:
```
daq_add_unit_test(ValueWrapper_test)
```
If you pass this function a name, e.g., `MyComponent_test`, it will create a unit test executable off of a source file called `sourcecode/<your packagename>/unittest/MyComponent_test.cxx`, and handle linking in the Boost unit test dependencies. You can also optionally have it link in other libraries by providing them after the `LINK_LIBRARIES` argument as in other functions; in the above example, this isn't needed because ValueWrapper is a template class which is instantiated within the unit test code itself.   

At the bottom of CMakeLists.txt, you'll see the following function:
```
daq_install()
```
When you call it it will install the targets (executables, shared object libraries) you wish to make available to others who want to use your package in a directory called `<your installation directory>/<pkgname>` (by default that would be `./install/toylibrary`). You'll also need to add a special file to your project for this function to work; this is discussed more fully in the "Installing your project as a local package" section later in this document. 

## In-depth documentation of the DUNE DAQ CMake functions

...can be found in the DAQ module file itself, in the comments above each function: https://github.com/DUNE-DAQ/daq-buildtools/blob/v1.2.1/cmake/DAQ.cmake

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

For starters, you'll want to call the DAQ module's `daq_install()` function at the bottom of your CMakeLists.txt file, as described earlier in this document. 

A major thing you should be aware of is that when you call CMake's `find_package` function, it will look for a file with the name `mypackageConfig.cmake` in a predetermined set of directories, including the one you defined (or allowed to default to `./install`) when you initially set up your development area as described elsewhere in the documentation. What a standard `mypackageConfig.cmake` file should look like with modern CMake is documented in many places on the web, but in order to make life as easy as possible there's a templatized version of this file in the daq-buildtools package. Assuming you've got a `./sourcecode/mypackage` repo in your development area, you can do the following:
```
cd ./sourcecode/mypackage
mkdir cmake
cp ../daq-buildtools/configs/Config.cmake.in cmake/mypackageConfig.cmake.in
```
and then let's look at the opening lines of `mypackageConfig.cmake.in`:
```

@PACKAGE_INIT@

include(CMakeFindDependencyMacro)

# Insert find_dependency() calls for your package's dependencies in                                                        
# the place of this comment. Make sure they match up with the                                                              
# find_package calls in your package's CMakeLists.txt file                                                                 
```
The only part of this file you need to worry about is the "Insert find_dependency()..." comment. In place of this comment, you'll want to call CMake's `find_dependency` function (details [here](https://cmake.org/cmake/help/latest/module/CMakeFindDependencyMacro.html)) for each package that mypackage depends on; this ensures that developers who call `find_package(mypackage)` don't need to have explicit `find_package` calls on these dependencies.

You can see an example of this kind of file with `./sourcecode/appfwk/cmake/appfwkConfig.cmake.in`.

Once you've edited this file as described, from the base of your development area you can then run 
```
./build_daq_software.sh --install 
```
without receiving an error message informing you that installation isn't an option. 

## Switching from daq-buildtools v1.1.0 to v1.2.1

As you've seen, there are a number of differences between how you'd build a package against daq-buildtools v1.1.0 vs. v1.2.1. The primary difference from the perspective of your project's CMakeLists.txt file is that it can now use a set of convenience functions to build its primary shared object library, its applications, etc. These functions are `daq_add_library`, `daq_add_plugin`, `daq_add_application`, and `daq_add_unit_test`. You'll need to read the in-file documentation of the convenience functions in `./sourcecode/daq-buildtools/cmake/DAQ.cmake` to fully understand what they offer, but a few things should be pointed out here:

* These functions make assumptions about paths and filenames of your source and header files which standard CMake functions (`add_library`, etc.) don't. E.g., `daq_add_library` wants source files' names given relative to your project's `./src` subdirectory; if you pass it `src/MyLib.cpp` as a source file it'll look for `src/src/MyLib.cpp`. 

* All these functions have a LINK_LIBRARIES argument which lets you link libraries without needing to separately call CMake's `target_link_libraries`. This includes `daq_add_unit_test`, which existed in v1.1.0 but didn't use this argument in v1.1.0. 

* They also take it upon themselves to perform the role of v1.1.0's now-removed `daq_point_build_to` function. E.g., `daq_add_library` will stick your library in `<your dev area>/build/<packagename>/src` without you needing to take any action. Note that some functions will look for your file in a different location depending on the arguments you provide; e.g. `daq_add_application` will look for source files in `test/apps/` if you provide it the `TEST` argument, and in `apps/` if you don't. 

* Similarly, the convenience functions will implicitly add their target to the list of things to install from your package. So, e.g., if you call `daq_add_library` in your CMakeLists.txt, you don't need to explicitly tell `daq_install()` to install it. 

* Not all use cases for desired targets are covered by the convenience functions. As new ones come up with relative frequency, new convenience functions can be added, but for the time being there are situations where you'll need to use standard CMake functions- e.g., if you want to export only headers in target form via an INTERFACE library. A downside of not using a convenience function is that you'll need to add these two lines to get your nonstandard target installed:
```
_daq_define_exportname()
install(TARGETS <your target's name> EXPORT ${DAQ_PROJECT_EXPORTNAME} )
```
&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;For future daq-buildtools versions we'll look into simplifying this, since the point of the DUNE DAQ functions is to reduce boilerplate, not add it. 

* The `<packagename>Config.cmake.in` file described earlier in this wiki has changed location, as you may have noticed: from the base of your repo to its `./cmake` subdirectory. Additionally, improvements have been made to the "template" `./sourcecode/daq-buildtools/configs/Config.cmake.in` to account for the fact that daq-buildtools v1.2.1 can support builds of multiple repositories at once, so you'll want to discard your existing `<packagename>Config.cmake.in` and step through the instructions in the "Installing your project as a local package" section of this wiki to recreate it. 

