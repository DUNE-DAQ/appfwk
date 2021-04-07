This page should contain the documentation for Queue interface. 

# Queue Interfaces and related objects

The overall diagram of the object used to move data between the DAQModules is 

![Queue UML](https://user-images.githubusercontent.com/31312964/85423669-e1f23b80-b56e-11ea-974f-f94d359333b9.png)

## `Queue` overview

This is the basic interface for all transport methods. 
Queue interfaces are quite simple: 
* they have a name used as a unique-ID inside a process to make all the connections
* they can `pop` and `push` objects
* they can check whether the previous methods can be performed based on the internal memory conditions of the queue. 

Note that `push` and `pop` are void. 
In order to check whether the operations succeeded they are supposed to be wrapped inside a `try - catch`. 
The thrown exception is defined in the `Queue.hpp` interface and it is a ers issue called `QueueTimeoutExpired`.

While the memory type transported by a queue is decided by the DAQModules, queue's types (`queue_kind`) are strongly controlled by the framework. 
Users are not suppose to extend the queue and the available queues are defined in the framework with a dedicated `enum`. 

## `QueueRegistry`

This is the singleton responsible to deliver to each DAQModule the queue based on the appropriate name. 
The service instantiates the queues based on their type and on the memory type to be exchanged between modules.
The queue's type is a configurable parameter and has to be one of those provided by the framework, the memory is specified by the module with a template request. 
In case two modules are connected by a queue with the same name but different type/memory  an ERS Issues is thrown. 

## Queue wrappers
The queue pointers shall never be directly visible from a `DAQModule` and in principle the `QueueRegistry` should not be called by the `DAQModulues` either, otherwise a DAQModule could have access to either `push` and `pop` of the same queue at the same time.  
In order to avoid this, queues have dedicated wrappers that retrieve the proper queue based on the name, and perform the transport operation exposing only selected interfaces. 
The wrappers are 
* `DAQSink` the one that exposes `push`
* `DAQSource` the one that exposes `pop`
Both are constructed simply using their name and will query the `QueueRegistry` to get the right `Queue` pointer. 