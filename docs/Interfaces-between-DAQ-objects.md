This page is hosting a description of how the different objects inside a DAQ process are supposed to interface to each other. At the moment nothing is definitive, this is simply reflecting the code status.

A DAQ Process is a collection of DAQModules that are connected by object called Queues. 
There are no restriction on the number of Modules a queue is connected to so the overall graph can scale to a high degree of complexity. 

Documentations on the specific objects can be find at these links:
* [[DAQModules|DAQModules]]
* [[Queues|Queue-interfaces]]
