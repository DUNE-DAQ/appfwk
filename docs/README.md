# appfwk: The DUNE DAQ Application Framework

## Overview

**In a single sentence:** 

appfwk consists of a generic DAQ application (`daq_application`) which can be configured to run different types of DAQ modules performing various DAQ-related tasks, where DAQ modules are user-developed implementations of appfwk's `DAQModule` interface class. 

**In a bit more detail:**

appfwk provides the scaffolding on which all DUNE DAQ software processes can be developed. The running DAQ typically consists of multiple distinct processes assigned various tasks: filtering data, requesting it, saving it to storage, etc. There are many different types of process, some of which may not even have been conceived of yet, and it would be cumbersome to recompile multiple different types of process across many packages every time one wanted to change the behavior of the DAQ. To solve this problem, the approach that's been taken is to have a standard DUNE DAQ software process [`daq_application`](Daq-Application.md) which can be configured at runtime by Run Control in order to perform some particular function in the DAQ. 

`daq_application` is designed as a flexible container of "DAQ modules" (units of code designed to perform specific tasks) and "queues" (designed to move data between DAQ modules). These specific tasks can vary widely; they include [producing fake data for testing purposes](https://github.com/DUNE-DAQ/readout/blob/develop/plugins/FakeCardReader.hpp), [putting data into long term storage](https://github.com/DUNE-DAQ/dfmodules/blob/develop/plugins/DataWriter.hpp), and so forth. DAQ modules will typically execute user-defined functions when receiving standard transitions from Run Control: "conf", "start", etc. appfwk provides the `DAQModule` base class which users should derive their DAQ module class from in their own packages.  

## Running `daq_application`

In general, in a full blown DAQ system users won't be running `daq_application` from the command line. However, it can be instructive to know what options `daq_application` takes. Details can be found [here](Daq-Application.md).

<a name="Writing_DAQ_modules"></a>
## Writing DAQ modules

### Basics of the `DAQModule` interface

When implenting a DAQ module, you'll want to `#include` the [`DAQModule.hpp` header](https://github.com/DUNE-DAQ/appfwk/blob/develop/include/appfwk/DAQModule.hpp), and derive your DAQ module from the `DAQModule` base class. The most important parts of `DAQModule.hpp` to an implementor of a DAQ module are the following:
* `DEFINE_DUNE_DAQ_MODULE`: This is a macro which should be "called" at the bottom of your DAQ module's source file with an "argument" of the form `dunedaq::<your_package_name>::<your DAQ module name>`. E.g., `DEFINE_DUNE_DAQ_MODULE(dunedaq::dfmodules::DataWriter)` [at the bottom of the dfmodules package's DataWriter module's source file](https://github.com/DUNE-DAQ/dfmodules/blob/develop/plugins/DataWriter.cpp) 
* `register_command`: takes as arguments the name of a command and a function which should execute when the command is received. The function is user defined, and takes an instance of `DAQModule::data_t` as argument. `DAQModule::data_t` is aliased to the `nlohmann::json` type and can thus be thought of as a blob of JSON-structured data. While in principle any arbitary name could be associated with any function of arbitrary behavior to create a command, in practice implementors of DAQ modules define commands associated with the DAQ's state machine: "_conf_", "_start_", "_stop_", "_scrap_". Not all DAQ modules necessarily need to perform an action for each of those transitions; e.g., a module may only be designed to do something during configuration, and not change as the DAQ enters the running state ("_start_") or exits it ("_stop_").  
* `init`: this pure virtual function's implementation is meant to create objects which are persistent for the lifetime of the DAQ module. It takes as an argument the type `DAQModule::data_t`. Typically it will use parameters from this JSON argument to define the persistent objects. For persistent objects of types which don't have an efficient copy assigment operator, a common technique is to declare as member data a `unique_ptr` to the type of interest and then, in `init`, to allocate the desired object on the heap using values from the JSON and point the `unique_ptr` member to it. Queues are commonly allocated in `init`; they'll be described in more detail later in this document. 

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

### Overview of additional function-by-function descriptions

A set of programming idioms have been developed over the first year of DAQ module development which, while not strictly necessary for implementing DAQ modules, have proven to be quite useful. They'll be described within the context of the functions above, as well as some new functions described later. While writers of DAQ modules should follow the coding guidelines presented in the [DUNE DAQ C++ Style Guide](https://dune-daq-sw.readthedocs.io/en/latest/packages/styleguide/), the techniques which will be presented below concern a higher-level view of the code and unlike the guidelines are more recommendations than requirements. Of course, you can also see the techniques in action by looking at the source code of actual DAQ modules. 

### The DAQ module's constructor

While of course all member data will be initialized here in a _technical_ (as opposed to logical) sense, in general the only things about a DAQ module instance which are meaningfully defined in its constructor are:
1. Its command set, already discussed
2. Its unique name, via the argument to its constructor

A word needs to be said about the concept of a "unique name" here. Looking in [`DAQModule.hpp`](https://github.com/DUNE-DAQ/appfwk/blob/develop/include/appfwk/DAQModule.hpp), you'll see that the `DAQModule` base class itself inherits from an appfwk class called [`NamedObject`](https://github.com/DUNE-DAQ/appfwk/blob/develop/include/appfwk/NamedObject.hpp). The instances of any class which inherits from `NamedObject` will require unique names; while this of course would include DAQ modules as they inherit from `DAQModule`, [other types of class can require unique names as well](https://github.com/DUNE-DAQ/dfmodules/blob/a91706c214f9ae7f1cca0840af7d0381569be83f/src/dfmodules/TriggerInhibitAgent.hpp).

### The `init` function

Already touched upon above, this function takes a `data_t` instance (i.e., JSON) to tell it what objects to make persistent over the DAQ module's lifetime. A very common example of this is the construction of the queues which will pipe data into and out of an instance of the DAQ module. A description of this common use case will illustrate a couple of very important aspects of DAQ module programming. 

When a DAQ module writer wants to bring data into a DAQ module, they'll want to pop data off a queue via the [`DAQSource` class](https://github.com/DUNE-DAQ/appfwk/blob/develop/include/appfwk/DAQSource.hpp), and when they want to send data out of a DAQ module they'll want to push it onto a queue via the [`DAQSink` class](https://github.com/DUNE-DAQ/appfwk/blob/develop/include/appfwk/DAQSink.hpp). Typically instances of these classes aren't "hardwired" into the DAQ module through creation in the constructor. Instead, they get built in the call to `init` based on the JSON configuration `init` receives . A definition of `init`, then, can look like the following:
```
void MyDaqModule::init(const data_t& init_data) {
    auto qi = appfwk::queue_index(init_data, {"name_of_required_input_queue"});
    m_required_input_queue_ptr.reset(new dunedaq::appfwk::DAQSource<MyType_t>(qi["name_of_required_input_queue"].inst));
}
```
In the code above, the call to `queue_index`, defined in [`DAQModuleHelper.cpp`](https://github.com/DUNE-DAQ/appfwk/blob/develop/src/DAQModuleHelper.cpp), returns a map which connects the names of queues with structs which reference the queues. It will throw an exception if any provided names don't appear - so in this case, if `name_of_required_input_queue` isn't found in `init_data`, an exception will be thrown. If the name is found, then `m_required_input_queue_ptr`, which here is an `std::unique` to a `DAQSource` of `MyType_t`s, gets pointed to a newly-allocated `DAQSource`. When the DAQ enters the running state, we could have `MyDaqModule` pop elements of `MyType_t` off of the queue pointed to by `m_required_input_queue_ptr` for processing. 

For a JSON file which (among other things) defines queues, see [this example](https://github.com/DUNE-DAQ/flxlibs/blob/15e256c0df102b1fc93802e9ed79a7cfd8c0ea4a/test/felix_wib2_readout.json), where the two main things defined in the JSON for a queue are (1) its capacity (the maximum number of elements it can hold) and (2) the kind of queue it is. The two primary queue options for DAQ running are "FollySPSCQueue" (Single Producer Single Consumer) and "FollyMPMCQueue" (Multiple Producer Multiple Consumer), both implemented originally for Facebook but found useful for DUNE. 

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
This of course raises the question: what _is_ `mydaqmodule::Conf`? It's a `struct`, but rather than being manually written the code for it is generated by the DUNE DAQ build system itself, using a `jsonnet` file as input. It's in the `jsonnet` file that the logical contents of the struct are defined; an example of this type of file can be found [here](https://github.com/DUNE-DAQ/listrev/blob/develop/schema/listrev/randomdatalistgenerator.jsonnet). This approach allows automatic compile-time checks on the variable (here `mydaqmodule::Conf`) retrieved by the module, reducing the workload on the implementor of `do_conf`. Note also that in fact many functions in a DAQ module, including `init`, can use JSON as input to control their actions, not just `do_conf`. Further details on the generation of code from `jsonnet` files are beyond the scope of appfwk documentation and are instead covered in [this section of the daq-cmake documentation](../daq-cmake/README.md#daq_cmake_schema).

### The `do_start` function

Most DAQ modules are designed to loop over some sort of repeated action when the DAQ enters the running state, and it's in the `do_start` function that this repeated action begins. A very common technique for the `do_start` function is, "Set an atomic boolean stating that we're now in the running state, and then start one or more threads which perform actions in loops which they break out of if they see that the atomic boolean indicates we're no longer in the running state". 

While it's of course possible to accomplish this using the existing concurrency facilities provided by the C++ Standard Library, the appfwk package itself provides a class, `ThreadHelper`, which makes this easier. `ThreadHelper` is covered in detail [here](https://dune-daq-sw.readthedocs.io/en/latest/packages/appfwk/ThreadHelper-Usage-Notes/); when in use the `do_start` function can be as simple as follows:
```
void MyDaqModule::do_start(const data_t& /*args*/) {
    m_thread.start_working_thread();  // m_thread is an `appfwk::ThreadHelper` member of MyDaqModule
}
```
Note that `start_working_thread` takes an optional argument which gives the `ThreadHelper` instance a name, potentially allowing shifters to keep track of various threads for debugging purposes. 

### The `do_stop` function

Quite simple, basically the reverse of `do_start`:
```
void MyDaqModule::do_stop(const data_t& /*args*/) {
    m_thread.stop_working_thread();  // m_thread is an `appfwk::ThreadHelper` member of MyDaqModule
}
```
Note that if your `do_start` function also allocates any resources (hardware, memory, etc.) it should be deallocated here. Also, the queues which send data to your DAQ module should be drained. The idea is that you want your DAQ module to be able to accept a "start" transition after receiving a "stop" transition without anything from the previous run interfering.  

### The `do_scrap` function

This is the reverse of `do_config`. Often this function isn't even needed since the values which get set in `do_conf` are completely overwritten on subsequent calls to `do_conf`. However, as the point of this function is to bring the DAQ module back to a state where it can be configured again, it's important that any hardware or memory resources which were acquired in `do_conf` are released here in `do_scrap`.  

### The `get_info` function

Not yet mentioned, you can see in [`DAQModule.hpp`](https://github.com/DUNE-DAQ/appfwk/blob/develop/include/appfwk/DAQModule.hpp) that there's a virtual function called `get_info` which defaults to a no-op:
```
virtual void get_info(opmonlib::InfoCollector& /*ci*/, int /*level*/) { return; };
```
It's meant to be implemented by DAQ module writers to supply metrics about the DAQ module; an example of this can be found [here](https://github.com/DUNE-DAQ/dfmodules/blob/develop/plugins/DataWriter.cpp). 

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
     std::unique_ptr<dunedaq::appfwk::DAQSource<MyType_t>> m_required_input_queue_ptr; 
};
```
* `MyDaqModule.cpp`:
```

void MyDaqModule::init(const data_t& init_data) {
    auto qi = appfwk::queue_index(init_data, {"name_of_required_input_queue"});
    m_required_input_queue_ptr.reset(new dunedaq::appfwk::DAQSource<MyType_t>(qi["name_of_required_input_queue"].inst));
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

void MyDaqModule::do_stop(const data_t& /*args*/) {
    m_thread.stop_working_thread();  
}

void MyDaqModule::do_scrap(const data_t& /*args*/) {
    m_calibration_scale_factor = -1;
}

void MyDaqModule::do_work(std::atomic<bool>& running_flag)
{
   while (running_flag.load()) {
      // Here we'd pop data off of the queue pointed at by m_required_input_queue_ptr and presumably use m_calibration_scale_factor when processing the data
   }
}   

DEFINE_DUNE_DAQ_MODULE(dunedaq::mypackage::MyDaqModule)
```

### Final thoughts on writing DAQ modules

Now that you've been given an overview of appfwk and how to write DAQ modules, you're encouraged to look at the various repos to see how other DUNE DAQ developers have written DAQ modules. One package with plenty of DAQ modules to study is [dfmodules](https://github.com/DUNE-DAQ/dfmodules/tree/develop/plugins), modules used for dataflow purposes. Keep in mind that not all DAQ modules will adhere to the model described above, and you can judge for yourself what techniques you feel will make it easiest to write and maintain a DAQ module. 


