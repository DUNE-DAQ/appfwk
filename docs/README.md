# appfwk: The DUNE DAQ Application Framework

## Overview

**In a single sentence:** 

appfwk consists of a generic DAQ application (`daq_application`) which can be configured to run different types of DAQ modules performing various DAQ-related tasks, where DAQ modules are user-defined implementations of appfwk's `DAQModule` interface class. 

**In a bit more detail:**

appfwk provides the scaffolding on which all standard DUNE DAQ software processes can be built. The running DAQ typically consists of multiple distinct processes assigned various tasks: filtering data, requesting it, saving it to storage, etc. There are many different types of process, some of which may not even have been conceived of yet, and it would be cumbersome at best to need to recompile multiple different types of process every time one wanted to change the behavior of the DAQ. To solve this problem, the approach that's been taken is to have a standard DUNE DAQ software process [`daq_application`](Daq-Application.md) which can be configured at runtime by Run Control in order to perform some particular function in the DAQ. 

Drilling down a bit deeper, the way that `daq_application`s can be configured so flexibly is that they are designed to have DAQ modules embedded in them, where DAQ modules are units of code designed to perform specific tasks and which passing data between each other by means of containers known as "queues". These specific tasks can vary widely; they include [producing fake data for testing purposes](https://github.com/DUNE-DAQ/readout/blob/develop/plugins/FakeCardReader.hpp), [sending requests for data upstream given the presence of a trigger](https://github.com/DUNE-DAQ/dfmodules/blob/develop/plugins/RequestGenerator.hpp), and so forth. appfwk provides the `DAQModule` base class, which is quite general and which various groups will typically implement in their own packages which depend on appfwk.  

## Running `daq_application`

In general, in a full blown DAQ system users won't be running `daq_application` from the command line. However, it can be instructive to know what options `daq_application` takes. Details can be found [here](Daq-Application.md).

## Writing DAQ modules: the basics

The parts of the [`DAQModule.hpp` header](https://github.com/DUNE-DAQ/appfwk/blob/develop/include/appfwk/DAQModule.hpp) most important to an implementor of a DAQ module are the following:
* The constructor, as this is where calls to `register_command` are typically made in order to define the DAQ module's command set
* `register_command`: takes as arguments the name of a command and associated function which should execute when the command is received, and which takes an instance of `DAQModule::data_t` as argument. `register command` is typically called multiple times in the DAQ module's constructor to define the functionality of the DAQ module. While in principle any arbitary name could be associated with any function of arbitrary behavior to create a command, in practice implementors of DAQ modules define commands associated with the DAQ's state machine: "conf", "start", "stop", "scrap". An idiom exists where the associated functions are private member functions with standardized names: `do_conf`, `do_start`, etc. 
* `init`: this pure virtual function's implementation is meant to initialize the DAQ module instance, i.e., build objects which are meant to be persistent across runs, for the lifetime of the DAQ module. It takes as an argument the type `DAQModule::data_t`, currently aliased to the `nlohmann::json` type - i.e., JSON is used to define the DAQ module's initialization. Typically it will use parameters from the JSON in order to de-facto construct the member data (which of course is in technically constructed in the DAQ module's constructor). A common idiom to achieve this is to declare a `unique_ptr` to a type of interest as a member datum and then, in `init`, allocate the desired object on the heap using values from the JSON, pointing the `unique_ptr` instance to it.

An conceptual example of what this looks like is the following simplified version of a DAQ module implementation. 
```
// This file would be called plugins/MyDAQModule.hpp
// Functions are defined would typically be defined in plugins/MyDAQModule.cpp

class MyDAQModule : public dunedaq::appfwk::DAQModule {
  public:
     MyDAQModule(const std::string& name) : 
        dunedaq::appfwk::DAQModule(name) {
          register_command("conf",  &MyDAQModule::do_conf);
          register_command("start", &MyDAQModule::do_start);
          register_command("stop",  &MyDAQModule::do_stop);
          register_command("scrap", &MyDAQModule::do_scrap);
     }
     
     void init(const data_t& data) override;
  
  private:
  
     // Typically would define these functions in `plugins/MyDAQModule.cpp`
     void do_conf(const data_t& conf_data);
     void do_start(const data_t& start_data);
     void do_stop(const data_t& stop_data);
     void do_scrap(const data_t& scrap_data);
};

```


## Reference Documentation

* [Glossary](Glossary-of-Terms)
