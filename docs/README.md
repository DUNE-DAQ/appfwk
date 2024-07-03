# appfwk: The DUNE DAQ Application Framework

## Overview

**In a single sentence:** 

appfwk consists of a generic DAQ application (`daq_application`) which can be configured to run different types of DAQ modules performing various DAQ-related tasks, where DAQ modules are user-developed implementations of appfwk's `DAQModule` interface class. 

**In a bit more detail:**

appfwk provides the scaffolding on which all DUNE DAQ software processes can be developed. The running DAQ typically consists of multiple distinct processes assigned various tasks: filtering data, requesting it, saving it to storage, etc. There are many different types of process, some of which may not even have been conceived of yet, and it would be cumbersome to recompile multiple different types of process across many packages every time one wanted to change the behavior of the DAQ. To solve this problem, the approach that's been taken is to have a standard DUNE DAQ software process [`daq_application`](Daq-Application.md) which can be configured at runtime by Run Control in order to perform some particular function in the DAQ. 

`daq_application` is designed as a flexible container of "DAQ modules" (units of code designed to perform specific tasks) and "connections" (designed to move data between DAQ modules that can be in the same or in different DAQ applications). These specific tasks can vary widely; they include [producing fake data for testing purposes](https://github.com/DUNE-DAQ/readoutmodules/blob/develop/plugins/FakeCardReader.hpp), [putting data into long term storage](https://github.com/DUNE-DAQ/dfmodules/blob/develop/plugins/DataWriterModule.hpp), and so forth. DAQ modules will typically execute user-defined functions when receiving standard transitions from Run Control: "conf", "start", etc. appfwk provides the `DAQModule` base class which users should derive their DAQ module class from in their own packages.  

![daq_application](https://github.com/DUNE-DAQ/appfwk/raw/develop/docs/Application.png)

## Running `daq_application`

In general, in a full blown DAQ system users won't be running `daq_application` from the command line. However, it can be instructive to know what options `daq_application` takes. Details can be found [here](Daq-Application.md).

<a name="Writing_DAQ_modules"></a>
## Writing DAQ modules

### Basics of the `DAQModule` interface

 <!--**_Be aware that much of the boilerplate code described below can be automatically generated using the [create_dunedaq_package script](https://dune-daq-sw.readthedocs.io/en/latest/packages/daq-cmake/#the-create_dunedaq_package-script)_** -->

When implenting a DAQ module, you'll want to `#include` the [`DAQModule.hpp` header](https://github.com/DUNE-DAQ/appfwk/blob/develop/include/appfwk/DAQModule.hpp), and derive your DAQ module from the `DAQModule` base class. The most important parts of `DAQModule.hpp` to an implementor of a DAQ module are the following:
* `DEFINE_DUNE_DAQ_MODULE`: This is a macro which should be "called" at the bottom of your DAQ module's source file with an "argument" of the form `dunedaq::<your_package_name>::<your DAQ module name>`. E.g., `DEFINE_DUNE_DAQ_MODULE(dunedaq::dfmodules::DataWriterModule)` [at the bottom of the dfmodules package's DataWriterModule module's source file](https://github.com/DUNE-DAQ/dfmodules/blob/develop/plugins/DataWriterModule.cpp) 
* `register_command`: takes as arguments the name of a command and a function which should execute when the command is received. The function is user defined, and takes an instance of `DAQModule::data_t` as argument. `DAQModule::data_t` is aliased to the `nlohmann::json` type and can thus be thought of as a blob of JSON-structured data. While in principle any arbitary name could be associated with any function of arbitrary behavior to create a command, in practice implementors of DAQ modules define commands associated with the DAQ's state machine: "_conf_", "_start_", "_stop_", "_scrap_". Not all DAQ modules necessarily need to perform an action for each of those transitions; e.g., a module may only be designed to do something during configuration, and not change as the DAQ enters the running state ("_start_") or exits it ("_stop_"). It also supports an optional third argument which lists the states that the application must be in for the command to be valid. [!!!Control People here should make comments and see if this is correct, if it's sitll the plan, etc]
* `init`: this pure virtual function's implementation is meant to create objects which are persistent for the lifetime of the DAQ module. It also has the unique role of connecting the DAQModel with its own configuration object, see later section [MR update this!!!]. It takes as an argument the type `std::shared_ptr<ModuleConfiguration>`. Typically, `init` will take the generic configuration object (`ModuleConfiguration`), cast it into an object specifically defined for this `DAQModule` and will store the pointer internally to the class for later usage, when the dedicated commands comes, usually `conf`. Connection objects are commonly allocated in `init`; they'll be described in more detail later in this document. 

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
     
     void init(std::shared_ptr<ModuleConfiguration>) override;
  
  private:
  
     void do_conf(const data_t& conf_data);
     void do_start(const data_t& start_data);
     void do_stop(const data_t& stop_data);
     void do_scrap(const data_t& scrap_data);

     const MyDAQModuleConf * m_cfg = nullptr;
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

Already touched upon above, this function takes a `std::shared_ptr<ModuleConfiguration>` instance to tell it what objects to make persistent over the DAQ module's lifetime. A very common example of this is the construction of the `iomanager` connections which will pipe data into and out of an instance of the DAQ module. A description of this common use case will illustrate a couple of very important aspects of DAQ module programming. 

When a DAQ module writer wants to communicate with other DAQ modules, they use the [`iomanager`](https://dune-daq-sw.readthedocs.io/en/latest/packages/iomanager/#connectionid-connectionref). The `iomanager` Sender and Receiver objects needed by a DAQ Module get built in the call to `init` based on the JSON configuration `init` receives . A definition of `init`, then, can look like the following:
```
void MyDaqModule::init(std::shared_ptr<ModuleConfiguration> p) {
    m_cfg = mcfg->module<MyDAQModuleConf>(get_name());
    if ( !m_cfg ) {
      throw appfwk::CommandFailed(ERS_HERE, "init", get_name(), "Unable to retrieve configuration object");
    }
    auto inputs = m_cfg->get_inputs();
    for (auto con : inputs) {
    if (con->get_data_type() == datatype_to_string <MyType1>()) {
      m_type1_con = con->UID();
    }
     if (con->get_data_type() == datatype_to_string<MyType2>()) {
      auto iom = iomanager::IOManager::get();
      m_type2_receiver = iom->get_receiver<MyType2>(con->UID());
    }
  }

}
```
In the code above, the configuration object is first, casted and then queried for the possible input connections. 
The information on the connection is used to decide what to use it for. The input of `MyType1` is simply used to store the name of the connection for later usage, while the inptu of `MyType2` is used to directly obtain the receiver socket from the `IOManager`.
Similar operations can be done on the outputs, for example see the [`TRBModule`](https://github.com/DUNE-DAQ/dfmodules/blob/2e9fc856e82cf566c2d38d024960a74cee910e75/plugins/TRBModule.cpp#L110). 
Of course in this case operations can be more complicatd because modules with multiple outputs of the same time might require a bit of more logic to organise where to send data. In that case ad-hoc solutions need to be adopted based on configuration schema object that is defined. 

This code of course raises the question: what _is_ `MyDAQModuleConf`? It's a `struct`, but rather than being manually written the code for it is generated by the DUNE DAQ build system itself, using a `oks` file schema as input. It's in the schema file that the logical contents of the struct are defined; an example of this type of file can be found [here](https://github.com/DUNE-DAQ/listrev/blob/develop/schema/listrev/listrev.schema.xml). This approach allows automatic compile-time checks on the variable (here `MyDAQModuleConf`) retrieved by the module, reducing the workload on the implementor of `do_conf` or other transitions.
[!!! Here some expert should decide what to do with this comment. Should we keep discussing jsonnet?!?!?]Note also that in fact many functions in a DAQ module, including `init`, can use JSON as input to control their actions, not just `do_conf`. Further details on the generation of code from `jsonnet` files are beyond the scope of appfwk documentation and are instead covered in [this section of the daq-cmake documentation](../daq-cmake/README.md#daq_cmake_schema).

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

While it's of course possible to accomplish this using the existing concurrency facilities provided by the C++ Standard Library, the `utilities` package provides a class, `WorkerThread`, which makes this easier. `WorkerThread` is covered in detail [here](https://dune-daq-sw.readthedocs.io/en/latest/packages/utilities/WorkerThread-Usage-Notes/); when in use the `do_start` function can be as simple as follows:
```
void MyDaqModule::do_start(const data_t& /*args*/) {
    m_thread.start_working_thread();  // m_thread is an `utilities::WorkerThread` member of MyDaqModule
}
```
Note that `start_working_thread` takes an optional argument which gives the `WorkerThread` instance a name, potentially allowing shifters to keep track of various threads for debugging purposes. 

### The `do_stop` function

Quite simple, basically the reverse of `do_start`:
```
void MyDaqModule::do_stop(const data_t& /*args*/) {
    m_thread.stop_working_thread();  // m_thread is an `utilities::WorkerThread` member of MyDaqModule
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
It's meant to be implemented by DAQ module writers to supply metrics about the DAQ module; an example of this can be found [here](https://github.com/DUNE-DAQ/dfmodules/blob/develop/plugins/DataWriterModule.cpp). 

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
     dunedaq::utilities::WorkerThread m_thread; 
     double m_calibration_scale_factor;
     std::shared_ptr<dunedaq::iomanager::Receiver<MyType_t>> m_required_input_ptr; 
};
```
* `MyDaqModule.cpp`:
```

void MyDaqModule::init(const data_t& init_data) {
    auto ci = appfwk::connection_index(init_data, {"name_of_required_input"});
    m_required_input_ptr = dunedaq::get_iom_receiver(ci["name_of_required_input"]));
}

void MyDaqModule::do_conf(const data_t& conf_data)
{
  auto data = conf_data.get<mydaqmodule::Conf>();

  m_calibration_scale_factor = data.calibration_scale_factor;
  // ...and then set the other members which take per-configuration values...
}

void MyDaqModule::do_start(const data_t& /*args*/) {
    m_thread.start_working_thread();  // m_thread is an `utilities::WorkerThread` member of MyDaqModule
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
      // Here we'd receive data from m_required_input_ptr and presumably use m_calibration_scale_factor when processing the data
   }
}   

DEFINE_DUNE_DAQ_MODULE(dunedaq::mypackage::MyDaqModule)
```

### Final thoughts on writing DAQ modules

Now that you've been given an overview of appfwk and how to write DAQ modules, you're encouraged to look at the various repos to see how other DUNE DAQ developers have written DAQ modules. The [listrev](https://github.com/DUNE-DAQ/listrev) package is maintained as an example of simple DAQ modules, and another package with plenty of real DAQ modules to study is [dfmodules](https://github.com/DUNE-DAQ/dfmodules/tree/develop/plugins), modules used for dataflow purposes. Keep in mind that not all DAQ modules will adhere to the model described above, and you can judge for yourself what techniques you feel will make it easiest to write and maintain a DAQ module. 


### API Diagram

![Class Diagrams](https://github.com/DUNE-DAQ/appfwk/raw/develop/docs/appfwk.png)
