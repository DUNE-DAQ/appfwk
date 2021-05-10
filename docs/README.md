# appfwk: The DUNE DAQ Application Framework

## Overview

**In a single sentence:** 

appfwk consists of a generic DAQ application (`daq_application`) which can be configured to run different types of DAQ modules performing various DAQ-related tasks, where DAQ modules are user-defined implementations of appfwk's `DAQModule` interface class. 

**In a bit more detail:**

appfwk provides the scaffolding on which all standard DUNE DAQ software processes can be built. The running DAQ typically consists of multiple distinct processes assigned various tasks: filtering data, requesting it, saving it to storage, etc. There are many different types of process, some of which may not even have been conceived of yet, and it would be cumbersome at best to need to recompile multiple different types of process every time one wanted to change the behavior of the DAQ. To solve this problem, the approach that's been taken is to have a standard DUNE DAQ software process [`daq_application`](Daq-Application.md) which can be configured at runtime by Run Control in order to perform some particular function in the DAQ. 

Drilling down a bit deeper, the way that `daq_application`s can be configured so flexibly is that they are designed to have DAQ modules embedded in them, where DAQ modules are units of code designed to perform specific tasks and which pass data between each other by means of containers known as "queues". These specific tasks can vary widely; they include [producing fake data for testing purposes](https://github.com/DUNE-DAQ/readout/blob/develop/plugins/FakeCardReader.hpp), [sending requests for data upstream given the presence of a trigger](https://github.com/DUNE-DAQ/dfmodules/blob/develop/plugins/RequestGenerator.hpp), and so forth. DAQ modules will typically execute user-defined functions when receiving standard transitions from Run Control: conf, start, etc. appfwk provides the `DAQModule` base class which users should derive their DAQ module class from in their own packages.  

## Running `daq_application`

In general, in a full blown DAQ system users won't be running `daq_application` manually from the command line. However, it can be instructive to know what options `daq_application` takes. Details can be found [here](Daq-Application.md).

## Writing DAQ modules: the basics

When implenting a DAQ module, you'll want to `#include` the [`DAQModule.hpp` header](https://github.com/DUNE-DAQ/appfwk/blob/develop/include/appfwk/DAQModule.hpp), and derive your DAQ module from the `DAQModule` base class. The most important parts of `DAQModule.hpp` to an implementor of a DAQ module are the following:
* `register_command`: takes as arguments the name of a command and a function which should execute when the command is received. The function is user defined, and takes an instance of `DAQModule::data_t` as argument. `DAQModule::data_t` currently aliased to the `nlohmann::json` type and can thus be thought of as a blob of JSON-structured data. `register command` is typically called multiple times in the DAQ module's constructor, once for each relevant Run Control transition, to define the functionality of the DAQ module. While in principle any arbitary name could be associated with any function of arbitrary behavior to create a command, in practice implementors of DAQ modules define commands associated with the DAQ's state machine: "_conf_", "_start_", "_stop_", "_scrap_". Not all DAQ modules necessarily need to perform an action for each of those transitions; e.g., a module may only be designed to do something during configuration, and not change as the DAQ enters the running state ("_start_") or exits it ("_stop_").  
* `init`: this pure virtual function's implementation is meant to initialize the DAQ module instance, i.e., build objects which are meant to be persistent for the lifetime of the DAQ module. It takes as an argument the type `DAQModule::data_t`. Typically it will use parameters from the JSON in order to de-facto construct the member data (which of course is in technically constructed in the DAQ module's constructor). A common idiom to achieve this is to declare a `unique_ptr` to a type of interest as a member datum and then, in `init`, allocate the desired object on the heap using values from the JSON, pointing the `unique_ptr` instance to it. Queues are commonly built in this function; queues are described in more detail later in this document. 

An conceptual example of what this looks like is the following simplified version of a DAQ module implementation. 
```
// This file would be called plugins/MyDaqModule.hpp
// Functions would typically be defined in plugins/MyDaqModule.cpp

class MyDaqModule : public dunedaq::appfwk::DAQModule {
  public:
     MyDaqModule(const std::string& name) : // A DAQ module instance is meant to have a unique name
        dunedaq::appfwk::DAQModule(name) {
          register_command("conf",  &MyDAQModule::do_conf);
          register_command("start", &MyDAQModule::do_start);
          register_command("stop",  &MyDAQModule::do_stop);
          register_command("scrap", &MyDAQModule::do_scrap);
     }
     
     void init(const data_t& init_data) override;
  
  private:
  
     // Typically would define these functions in `plugins/MyDaqModule.cpp`
     void do_conf(const data_t& conf_data);
     void do_start(const data_t& start_data);
     void do_stop(const data_t& stop_data);
     void do_scrap(const data_t& scrap_data);
};
```
A set of programming idioms have developed over the last year of DAQ module development which, while not strictly necessary for implementing DAQ modules, have proven to be quite useful. They'll be described within the context of the functions above. Of course, you can also see them in action by looking at the source code of actual DAQ modules. 

### The _init_ function

Already touched upon above, this function takes a `data_t` instance (i.e., JSON) to tell it what objects to make persistent over the DAQ module's lifetime. A very common example of this is the construction of the queues which will pipe data into and out of an instance of the DAQ module. A description of this common use case will illustrate a couple of very important aspects of DAQ module programming. 

When a DAQ module writer wants to bring data into a DAQ module, they'll want to pop data off a queue via the [`DAQSource` class](https://github.com/DUNE-DAQ/appfwk/blob/develop/include/appfwk/DAQSource.hpp), and when they want to send data out of a DAQ module they'll want to push it onto a queue via the [`DAQSink` class](https://github.com/DUNE-DAQ/appfwk/blob/develop/include/appfwk/DAQSink.hpp). Typically instances of these classes aren't "hardwired" into the DAQ module through creation in the constructor, but rather get built in the call to `init` based on the JSON configuration `init` receives. A definition of `init`, then, can look like the following:
```
void MyDaqModule::init(const data_t& init_data) {
    auto qi = appfwk::queue_index(init_data, {"name_of_required_input_queue"});
    m_required_input_queue.reset(new dunedaq::appfwk::DAQSource<MyType>(qi["name_of_required_input_queue"].inst));
}
```
In the code above, the call to `queue_index`, defined in [`DAQModuleHelper.cpp`](https://github.com/DUNE-DAQ/appfwk/blob/abd89ed3cba1f934d9df555727a1bf97d555d11e/src/DAQModuleHelper.cpp), returns a map which connects the names of queues with structs which reference the queues. It will throw an exception if any provided names don't appear - so in this case, if `name_of_required_input_queue` isn't found in `init_data`, an exception will be thrown. If that doesn't happen, then `m_required_input_queue`, which here is an `std::unique` to a `DAQSource` of `MyType`s, gets pointed to a newly-allocated `DAQSource`. When the DAQ enters the running state, we could have `MyDaqModule` pop elements of `MyType` off of `m_required_input_queue` for processing. 

## Reference Documentation

* [Glossary](Glossary-of-Terms)
