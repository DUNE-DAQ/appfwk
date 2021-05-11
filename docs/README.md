# appfwk: The DUNE DAQ Application Framework

## Overview

**In a single sentence:** 

appfwk consists of a generic DAQ application (`daq_application`) which can be configured to run different types of DAQ modules performing various DAQ-related tasks, where DAQ modules are user-developed implementations of appfwk's `DAQModule` interface class. 

**In a bit more detail:**

appfwk provides the scaffolding on which all DUNE DAQ software processes can be developed. The running DAQ typically consists of multiple distinct processes assigned various tasks: filtering data, requesting it, saving it to storage, etc. There are many different types of process, some of which may not even have been conceived of yet, and it would be cumbersome to recompile multiple different types of process across many packages every time one wanted to change the behavior of the DAQ. To solve this problem, the approach that's been taken is to have a standard DUNE DAQ software process [`daq_application`](Daq-Application.md) which can be configured at runtime by Run Control in order to perform some particular function in the DAQ. 

Drilling down a bit deeper, the way that `daq_application`s can be configured so flexibly is that they are designed to have DAQ modules embedded in them, where DAQ modules are units of code designed to perform specific tasks and which pass data between each other by means of containers known as "queues". These specific tasks can vary widely; they include [producing fake data for testing purposes](https://github.com/DUNE-DAQ/readout/blob/develop/plugins/FakeCardReader.hpp), [putting data into long term storage](https://github.com/DUNE-DAQ/dfmodules/blob/develop/plugins/HDF5DataStore.hpp), and so forth. DAQ modules will typically execute user-defined functions when receiving standard transitions from Run Control: conf, start, etc. appfwk provides the `DAQModule` base class which users should derive their DAQ module class from in their own packages.  

## Running `daq_application`

In general, in a full blown DAQ system users won't be running `daq_application` from the command line. However, it can be instructive to know what options `daq_application` takes. Details can be found [here](Daq-Application.md).

## Writing DAQ modules: the basics

When implenting a DAQ module, you'll want to `#include` the [`DAQModule.hpp` header](https://github.com/DUNE-DAQ/appfwk/blob/develop/include/appfwk/DAQModule.hpp), and derive your DAQ module from the `DAQModule` base class. The most important parts of `DAQModule.hpp` to an implementor of a DAQ module are the following:
* `DEFINE_DUNE_DAQ_MODULE`: This is a macro which should be "called" at the bottom of your DAQ module's source file with an "argument" of the form `dunedaq::<your_package_name>::<your DAQ module name>`. E.g., `DEFINE_DUNE_DAQ_MODULE(dunedaq::dfmodules::DataGenerator)` [at the bottom of the dfmodules package's source file](https://github.com/DUNE-DAQ/dfmodules/blob/develop/plugins/DataGenerator.cpp) 
* `register_command`: takes as arguments the name of a command and a function which should execute when the command is received. The function is user defined, and takes an instance of `DAQModule::data_t` as argument. `DAQModule::data_t` is aliased to the `nlohmann::json` type and can thus be thought of as a blob of JSON-structured data. While in principle any arbitary name could be associated with any function of arbitrary behavior to create a command, in practice implementors of DAQ modules define commands associated with the DAQ's state machine: "_conf_", "_start_", "_stop_", "_scrap_". Not all DAQ modules necessarily need to perform an action for each of those transitions; e.g., a module may only be designed to do something during configuration, and not change as the DAQ enters the running state ("_start_") or exits it ("_stop_").  
* `init`: this pure virtual function's implementation is meant to create objects which are meant to be persistent for the lifetime of the DAQ module. It takes as an argument the type `DAQModule::data_t`. Typically it will use parameters from this JSON argument to define the persistent objects. For persistent objects of types which don't have an efficient copy assigment operator, a common idiom to declare as member data a `unique_ptr` to the type of interest and then, in `init`, to allocate the desired object on the heap using values from the JSON and point the `unique_ptr` member to it. Queues are commonly allocated in `init`; they'll be described in more detail later in this document. 

An conceptual example of what this looks like is the following simplified version of a DAQ module implementation. 
```
// This file would be called plugins/MyDaqModule.hpp
// Functions would typically be defined in plugins/MyDaqModule.cpp

class MyDaqModule : public dunedaq::appfwk::DAQModule {
  public:
     MyDaqModule(const std::string& name) : // A DAQ module instance is meant to have a unique name
        dunedaq::appfwk::DAQModule(name)
          {
          register_command("conf",  &MyDAQModule::do_conf);
          register_command("start", &MyDAQModule::do_start);
          register_command("stop",  &MyDAQModule::do_stop);
          register_command("scrap", &MyDAQModule::do_scrap);
     }
     
     void init(const data_t& init_data) override;
  
  private:
  
     void do_conf(const data_t& conf_data);
     void do_start(const data_t& start_data);
     void do_stop(const data_t& stop_data);
     void do_scrap(const data_t& scrap_data);
};
```
A set of programming idioms developed over the first year of DAQ module development which, while not strictly necessary for implementing DAQ modules, have proven to be quite useful. They'll be described within the context of the functions above. Of course, you can also see them in action by looking at the source code of actual DAQ modules. 

### The `init` function

Already touched upon above, this function takes a `data_t` instance (i.e., JSON) to tell it what objects to make persistent over the DAQ module's lifetime. A very common example of this is the construction of the queues which will pipe data into and out of an instance of the DAQ module. A description of this common use case will illustrate a couple of very important aspects of DAQ module programming. 

When a DAQ module writer wants to bring data into a DAQ module, they'll want to pop data off a queue via the [`DAQSource` class](https://github.com/DUNE-DAQ/appfwk/blob/develop/include/appfwk/DAQSource.hpp), and when they want to send data out of a DAQ module they'll want to push it onto a queue via the [`DAQSink` class](https://github.com/DUNE-DAQ/appfwk/blob/develop/include/appfwk/DAQSink.hpp). Typically instances of these classes aren't "hardwired" into the DAQ module through creation in the constructor, but rather get built in the call to `init` based on the JSON configuration `init` receives. A definition of `init`, then, can look like the following:
```
void MyDaqModule::init(const data_t& init_data) {
    auto qi = appfwk::queue_index(init_data, {"name_of_required_input_queue"});
    m_required_input_queue.reset(new dunedaq::appfwk::DAQSource<MyType_t>(qi["name_of_required_input_queue"].inst));
}
```
In the code above, the call to `queue_index`, defined in [`DAQModuleHelper.cpp`](https://github.com/DUNE-DAQ/appfwk/blob/abd89ed3cba1f934d9df555727a1bf97d555d11e/src/DAQModuleHelper.cpp), returns a map which connects the names of queues with structs which reference the queues. It will throw an exception if any provided names don't appear - so in this case, if `name_of_required_input_queue` isn't found in `init_data`, an exception will be thrown. If that doesn't happen, then `m_required_input_queue`, which here is an `std::unique` to a `DAQSource` of `MyType_t`s, gets pointed to a newly-allocated `DAQSource`. When the DAQ enters the running state, we could have `MyDaqModule` pop elements of `MyType_t` off of `m_required_input_queue` for processing. 

### The `do_conf` function

As one might expect, there are many values which a DAQ module may rely on to perform its calculations when in the running state that ideally should be settable during the `conf` transition. The typical technique is to have some member data which in the DAQ module constructor intentionally gets initialized either to zero or to implausible values (e.g. `m_calibration_scale_factor(-1)`, `m_num_total_warnings(0)`) and then to set them properly during the `config` transition. You'll see in the code below that the type of the data instance `data` which gets extracted from the JSON is `mydaqmodule::Conf`, and then `data` is used to set the member(s). 
```
void MyDaqModule::do_conf(const data_t& conf_data)
{
  auto data = conf_data.get<mydaqmodule::Conf>();

  m_calibration_scale_factor = data.calibration_scale_factor;
  // ...and then set the other members which take per-configuration values...
}
```
This of course raises the question: what _is_ `mydaqmodule::Conf`? It's a `struct`, but rather than being manually written the code for it is generated by the DUNE DAQ build system itself, using a `jsonnet` file as input. It's in the `jsonnet` file that the logical contents of the struct are defined; an example of this type of file can be found [here](https://github.com/DUNE-DAQ/listrev/blob/develop/schema/listrev/randomdatalistgenerator.jsonnet). Further details are beyond the scope of appfwk documentation and are instead covered in the [daq-cmake documentation](https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/).

### The `do_start` function

Most DAQ modules are designed to loop over some sort of repeated action when the DAQ enters the running state, and it's in the `do_start` function that this repeated action begins. A very common idiom for the `do_start` function is, "Set a member atomic boolean stating that we're now in the running state, and then start one or more threads which perform actions in loops which they break out of if they see that the atomic boolean indicates we're no longer in the running state". 

While it's of course possible to accomplish this using the existing concurrency facilities provided by the C++ Standard Library, the appfwk package itself provides a class, `ThreadHelper`, which makes this easier. `ThreadHelper` is covered in detail [here](https://dune-daq-sw.readthedocs.io/en/latest/packages/appfwk/ThreadHelper-Usage-Notes/); when in use the `do_start` function can be as simple as follows:
```
void MyDaqModule::do_start(const data_t& /*args*/) {
    m_thread.start_working_thread();  // m_thread is an `appfwk::ThreadHelper` member of MyDaqModule
}
```

### The `do_stop` function

Quite simple, basically the reverse of `do_start`:
```
void MyDaqModule::do_stop(const data_t& /*args*/) {
    m_thread.stop_working_thread();  // m_thread is an `appfwk::ThreadHelper` member of MyDaqModule
}
```
### The `do_scrap` function

This is the reverse of `do_config`. Often this function isn't even needed since the values which get set in `do_conf` are completely overwritten on subsequent calls to `do_conf`

### The full code

Given the code features described above, `MyDaqModule` would look something like the following, ignoring things irrelevant to the pedagogy presented here, like proper error handling, log statements, `#include`s, etc. Pretend the name of the package `MyDaqModule` in is "mypackage":

* `MyDaqModule.hpp`:
```
class MyDaqModule : public dunedaq::appfwk::DAQModule {
  public:
     
     alias MyType_t = double; // Pretend this module processes an incoming stream of doubles 
    
     MyDaqModule(const std::string& name) : // A DAQ module instance is meant to have a unique name
        dunedaq::appfwk::DAQModule(name),
        m_thread(std::bind(&MyDaqModule::do_work, this, std::placeholders::_1)),
        m_calibration_scale_factor(-1)
        {
          register_command("conf",  &MyDAQModule::do_conf);
          register_command("start", &MyDAQModule::do_start);
          register_command("stop",  &MyDAQModule::do_stop);
          register_command("scrap", &MyDAQModule::do_scrap);
     }
     
     void init(const data_t& init_data) override;
  
  private:
  
     void do_conf(const data_t& conf_data);
     void do_start(const data_t& start_data);
     void do_stop(const data_t& stop_data);
     void do_scrap(const data_t& scrap_data);
     
     void do_work(std::atomic<bool>&);
     dunedaq::appfwk::ThreadHelper m_thread; 
     double m_calibration_scale_factor;
     std::unique_ptr<dunedaq::appfwk::DAQSource<MyType_t>> m_required_input_queue; 
};
```
* `MyDaqModule.cpp`:
```

void MyDaqModule::init(const data_t& init_data) {
    auto qi = appfwk::queue_index(init_data, {"name_of_required_input_queue"});
    m_required_input_queue.reset(new dunedaq::appfwk::DAQSource<MyType_t>(qi["name_of_required_input_queue"].inst));
}

void MyDaqModule::do_conf(const data_t& conf_data)
{
  auto data = conf_data.get<mydaqmodule::Conf>();

  m_calibration_scale_factor = data.calibration_scale_factor;
  // ...and then set the other members which take per-configuration values...
}

void MyDaqModule::do_start(const data_t& /*args*/) {
    m_thread.start_working_thread();  // m_thread is an `appfwk::ThreadHelper` member of MyDaqModule
}

void MyDaqModule::do_work(std::atomic<bool>& running_flag)
{
   while (running_flag.load()) {
      // Here we'd pop data off of m_required_input_queue and presumably use m_calibration_scale_factor when processing it
   }
}   

DEFINE_DUNE_DAQ_MODULE(dunedaq::mypackage::MyDaqModule)
```

### Final thoughts on writing DAQ modules

Now that you've been given an overview of appfwk and how to write DAQ modules, you're encouraged to look at the various repos to see how other DUNE DAQ developers have written DAQ modules. One package with plenty of DAQ modules to study is [dfmodules](https://github.com/DUNE-DAQ/dfmodules/tree/develop/plugins), modules used for dataflow purposes. Keep in mind that not all DAQ modules will adhere to the model described above, and you can judge for yourself what techniques you feel will make it easiest to write and maintain a DAQ module. 




## Reference Documentation

* [Glossary](Glossary-of-Terms)
