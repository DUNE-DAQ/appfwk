# appfwk: The DUNE DAQ Application Framework

## Overview

**In a single sentence:** 

appfwk consists of a generic DAQ application (`daq_application`) which can be configured to run different types of DAQ modules performing various DAQ-related tasks, where DAQ modules are user-defined implementations of appfwk's `DAQModule` interface class. 

**In a bit more detail:**

appfwk provides the scaffolding on which all standard DUNE DAQ software processes can be built. The running DAQ typically consists of multiple distinct processes assigned various tasks: filtering data, requesting it, saving it to storage, etc. There are many different types of process, some of which may not even have been conceived of yet, and it would be cumbersome at best to need to recompile multiple different types of process every time one wanted to change the behavior of the DAQ. To solve this problem, the approach that's been taken is to have a standard DUNE DAQ software process [`daq_application`](Daq-Application.md) which can be configured at runtime by Run Control in order to perform some particular function in the DAQ. 

Drilling down a bit deeper, the way that `daq_application`s can be configured so flexibly is that they are designed to have DAQ modules embedded in them, where DAQ modules are units of code designed to perform specific tasks and which passing data between each other by means of containers known as "queues". These specific tasks can vary widely; they include [producing fake data for testing purposes](https://github.com/DUNE-DAQ/readout/blob/develop/plugins/FakeCardReader.hpp), [sending requests for data upstream given the presence of a trigger](https://github.com/DUNE-DAQ/dfmodules/blob/develop/plugins/RequestGenerator.hpp), and so forth. appfwk provides the `DAQModule` base class, which is quite general and which various groups will typically implement in their own packages which depend on appfwk.  

## Running `daq_application`

In general, in a full blown DAQ system users won't be running `daq_application` from the command line. However, it can be instructive to know what options `daq_application` takes. Details can be found [here](Daq-Application.md).

## Writing DAQ modules

The parts of the [`DAQModule.hpp` header](https://github.com/DUNE-DAQ/appfwk/blob/develop/include/appfwk/DAQModule.hpp) most important to an implementor of a DAQ module are the following:
* `init`: this pure virtual function is meant to initialize the DAQ module instance. It takes as an argument the type `DAQModule::data_t`, currently aliased to the `nlohmann::json` type - i.e., JSON is used to define the DAQ module's initialization. Typically it will use parameters from the JSON in order to de-facto construct the member data (e.g., `m_member_unique_ptr_to_a_widget_type = std::make_unique<MyModule::WidgetType>(parameter_for_widget_construction_found_in_JSON);`)
* `register_command`: takes as arguments the name of a command and associated function which should execute when the command is received, and which takes an instance of `DAQModule::data_t` as argument. `register command` is typically called multiple times in the DAQ module's constructor to define the functionality of the DAQ module. While in principle any arbitary name could be associated with any function of arbitray behavior to create a command, in practice implementors of DAQ modules define commands associated with the DAQ's state machine: "conf", "start", "stop", "scrap". An idiom exists where the associated functions are private member functions with standardized names: `do_conf`, `do_start`, etc. 


***
* [Step-by-step instructions for creating your own DAQModule package under v2.0.0](Step-by-step-instructions-for-creating-your-own-DAQModule-package-under-v2.0.0) (_under construction_)
* [Step-by-step instructions for creating your own DAQModule package under v1.1.0](Step-by-step-instructions-for-creating-your-own-DAQModule-package-under-v1.1.0)
* [Step-by-step instructions for creating your own DAQModule package under v1.0.0](Step-by-step-instructions-for-creating-your-own-DAQModule-package-under-v1)
* [Step-by-step instructions for creating your own DAQModule package](Step-by-step-instructions-for-creating-your-own-DAQModule-package)

***

* [How to write your DAQModule](How-to-write-your-DAQModule)
* [Application Framework Contributor's Guide](Contributors-Guide)

## Reference Documentation

* [Glossary](Glossary-of-Terms)
* [Coding style](Coding-style)
* [Interfaces](Interfaces-between-DAQ-objects)
* [Testing](Testing)
