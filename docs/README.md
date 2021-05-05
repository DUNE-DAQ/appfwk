# appfwk: The DUNE DAQ Application Framework

**In a single sentence:** appfwk consists of a generic DAQ application (`daq_application`) which can be configured to run different types of DAQ modules performing various DAQ-related tasks, where DAQ modules are user-defined implementations of appfwk's `DAQModule` interface class. 

**In a bit more detail:**

appfwk provides the scaffolding on which all standard DUNE DAQ software processes can be built. The running DAQ typically consists of multiple distinct processes assigned various tasks: filtering data, requesting it, saving it to storage, etc. There are many different types of process, some of which may not even have been conceived of yet, and it would be cumbersome at best to need to recompile multiple different types of process every time one wanted to change the behavior of the DAQ. To solve this problem, the approach that's been taken is to have a standard DUNE DAQ software process [`daq_application`](Daq-Application.md) which can be configured at runtime by Run Control in order to perform some particular function in the DAQ. 

Drilling down a bit deeper, the way that `daq_application`s can be configured so flexibly is that they are designed to have DAQ modules embedded in them, where DAQ modules are units of code designed to perform specific tasks and which passing data between each other by means of containers known as "queues". These specific tasks can vary widely; they include [producing fake data for testing purposes](https://github.com/DUNE-DAQ/readout/blob/develop/plugins/FakeCardReader.hpp), [sending requests for data upstream given the presence of a trigger](https://github.com/DUNE-DAQ/dfmodules/blob/develop/plugins/RequestGenerator.hpp), and so forth. appfwk provides the `DAQModule` base class, which is quite general and which various groups will typically implement in their own packages which depend on appfwk. How to implement an instance of `DAQModule` in order to serve your own group's needs in described later in this document. 


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
