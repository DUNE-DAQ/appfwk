This page should contain the documentation for `DAQModule` interface. 

![DAQModule UML](https://user-images.githubusercontent.com/31312964/100124441-8f95ad80-2e73-11eb-862c-076b46d72ea7.png)

The `DAQModule` is the main insertion point for user-written code. 
Each `DAQModule` should be self-contained and focused on a single task. 
"Self-contained" means that it should not rely on the presence of any other DAQModules in the same application.
The "Single task" can be as complex as necessary to achieve the desired functionality. 

The `DAQModule` interface has the role of associating each command to an operation. 
The association is configurable but eventually each command has to be associated to a single member function  that
* returns a void
* accepts as an input a `const nlohmann::json &` from which to extract command specifics.

The association is done via the method `register_command` that every module calls during initialisation phase so that proper command response could be associated to each command name. Specifically the initialisation phase can be in the constructor or in the init method depending if the operation requires configuration input or not. 

Whenever a DAQModule is created by the DAQModuleManager, the DAQModuleManager initiates a first configuration operation calling `init()`.

## Ownership and instantiation
Each DAQModule is instantiated by the `DAQModeuleManager` and they are owned by it for all the lifetime of the application. 
The *appframework* does not limit the DAQModules that can be instantiated as it is expected that every package depending on the framework will add their own modules. 


