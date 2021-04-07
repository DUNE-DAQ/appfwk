This page defines the current language being used to describe elements of the DAQ Application Framework.

## Core Terms or General Concepts 
* **DAQ System**: A set of DAQ Applications working in concert to perform the functions needed to effect data acquisition.
* **DAQ Application**: A single unit of the DAQ System in the form of an executable program that is started and controlled through the CCM. DAQ Applications perform a defined set of tasks based on their configuration.
* **Interface**: A pure abstract base class.
* **Plugin**: A piece of code provided either by the toolkit or by users (generally-useful plugins should be incorporated into the toolkit if possible), characterized by a specific interface, to perform a single task. Multiple plugins with different implementations can exist for a single interface. Users of plugins should be shielded from any implementation details. The plugin compiled code should be a shared library.
* **Data Flow Programming**: (DFP) a paradigm to describe a larger system as a graph in terms of data transformation nodes and data transportation edges
* **DFP Node**: a unit of code with an identifier (name) unique in the system with zero or more **DFP Ports**
* **DFP Port**: an identified point on a **DFP Node** which may produce or accept DFP data message (or both).  It typically as a **Socket**
* **DFP Edge**: a data transport conduit between two **DFP Ports**
* **Socket**: the endpoints of a *DFP Edge*, each associated with a **DFP Port**.  An application may `send()` a message to a **Socket** and it may `recv()` a message from a **Socket** (eg: BSD sockets, ZeroMQ sockets)
* **Transport**: act of transmitting a message between a **Socket** `send()` to another **Socket** `recv()`, transports include transmission over TCP/IP, Unix domain sockets ("FIFO"s) and inter-thread via shared memory.

## Implementation definitions 
* **DAQModule**: A distinct set of code which performs a specific task. Implemented by users, will be loaded into a DAQ Application according to the application configuration. Can load plugins. Connected to other User Modules within a DAQ Application via queues.
* **DAQProcess**: A central class which will be called from a program's main function, responsible for loading core services and reading the Module List for that program. Every *DAQProcess* has a unique name and it is spawned by the CCM.
* **GraphConstructor**: A class which is responsible for loading the set of *DAQModules* and *Queues* needed for a given DAQ Application, as well as linking them together. It returns the loaded modules and buffers to the *DAQProcess*
* **Component** an implementation of one or more **Interfaces**. Components shall be instantiated via a Factory mechanism,via their *component type name* and an *instance name*. 
* **Factory** a component with restricted construct-ability that provides instantiated *Components*. 
* **Component Type Name** string data (not code) identifying a *Component* type, may be the C++ type name or another.
* **Component Instance Name** string data identifying one instantiation of a component.
* **Queue**: A class which moves data between User Modules within a DAQ Application and provides methods for storage and retrieval. May provide a queue-like interface, random access or other functionality but at the moment only a basic queue functionality is implemented. Every queue implementation needs to be thread safe.
