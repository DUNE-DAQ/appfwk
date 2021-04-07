This page contains a starting point to write a new DAQ Module compatible withe operation of the application framework. 

# Code
From the C++ point of view, DAQ modules are implementation of a [[DAQ Module interface|DAQModules]]. 
That means that apart from the constructor (that receives a name) only one method has to be implemented: `init()`.  
The function has to handle all those configuration items that are not supposed to be reconfigurable during the run. 
Optionally it can configure all those variables that can be overridden during run time.

The most important thing that `init` has to do is to call `register_command` in order to associate functions to each command that the module is expected to support. 
Each function has to be a set of operations that changes the internal status of the module according to the command that it is implementing. 
The prototype of each function is in the form 
```C++
void do_something( const vector<string> & args ) ;
```
The function can be virtual and the system is able to pick the most downstream specification. 

Assuming that the name of the module you are developing is `MyNewDAQModule` a call to associate the command `"start"` to a function called `do_start` will be 
```C++
register_command("start",  & MyNewDAQModule::do_start);
```


It is expected that the operations of the DAQ Module are carried on in other threads under the control of the DAQModule itself. 

## Good practices
Every piece of code that is related to the application framework will be part of a large suite of software that will require long term maintenance and support given the long lifetime of the DUNE experiment.
It is important that the [style guide](https://github.com/DUNE-DAQ/styleguide/blob/develop/dune-daq-cppguide.md) is respected.

<!--
 ## Thread Helper
Writing thread safe code can be disorienting. 
The application framework provides a utility called [ThreadHelper](https://github.com/DUNE-DAQ/appfwk/blob/develop/include/appfwk/ThreadHelper.hpp) that can be used in order to facilitate the Module development.
-->

# Compilation
DAQModules need to be created with a factory pattern simply via their name.
In the application framework this is dune using [CETlib](https://github.com/DUNE-DAQ/appfwk/wiki/Third-Party-Software). 
In order to do that, each implementation (`cpp` file) needs to have a call to the macro `DEFINE_DUNE_DAQ_MODULE(<name_including_namespace>)` for example
```C++
DEFINE_DUNE_DAQ_MODULE(dunedaq::appfwk::DummyModule)
```
In order to generate a shared object library that is linkable in runtime from the application framework, the name of the library has to be the in the form `<package>_<module_name_no_namespace>_duneDAQModule`.
This can be achieved simply adding a line in the `CMakeLists.txt` file of your project in the form `add_library(<package>_<module_name_no_namespace>_duneDAQModule  path/to/my/file.cpp)`. 
For example:
```CMake
add_library(appfwk_DummyModule_duneDAQModule test/DummyModule.cpp)
```