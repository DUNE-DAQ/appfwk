# Action Plans

## Overview

An ActionPlan defines a series of steps consisting of groups of modules, which are executed in response to a command from CCM. Groups of modules are defined either by module class or by module instances, and the execution of each step is in parallel by default, but can be changed to serial execution if needed. Each ActionPlan is associated with a FSMCommand object, and is run by the appliction when it recieves the corresponding command. If a command is received and no ActionPlan is defined, the application currently runs a "dummy" ActionPlan consisting of a single step where modules with the command registered are all run in parallel.

Action Plans allow for much finer-grained control over the execution of a command within an application, allowing for modules that have dependencies on one another to execute their commands correctly. It also introduces parallelization of command execution within each step, which helps with certain time-consuming module commands (e.g. configuring hardware on a link). The current implmentation uses std::future objects and a catch-all threading pattern to ensure that errors executing steps within an action plan do not lead to program crashes.

## Defining an ActionPlan

ActionPlans are defined in configuration using these objects:

```XML
 <class name="ActionPlan" description="A set of steps for an application to carry out a command">
  <attribute name="execution_policy" description="How the application should dispatch the command to modules matched by each step of the ActionPlan" type="enum" range="modules-in-parallel,modules-in-series" init-value="modules-in-parallel"/>
  <relationship name="command" class-type="FSMCommand" low-cc="one" high-cc="one" is-composite="no" is-exclusive="no" is-dependent="no"/>
  <relationship name="steps" class-type="DaqModulesGroup" low-cc="zero" high-cc="many" is-composite="no" is-exclusive="no" is-dependent="no"/>
 </class>


 <class name="DaqModulesGroup" description="Represents a group of DAQ Modules that can run commands in parallel or in series as one of the steps of an ActionPlan." is-abstract="yes">
 </class>

 <class name="DaqModulesGroupById">
  <superclass name="DaqModulesGroup"/>
  <relationship name="modules" class-type="DaqModule" low-cc="one" high-cc="many" is-composite="no" is-exclusive="no" is-dependent="no"/>
 </class>

 <class name="DaqModulesGroupByType">
  <superclass name="DaqModulesGroup"/>
  <attribute name="modules" type="class" is-multi-value="yes"/>
 </class>
```

1. ActionPlan relates a set of DAQModule groups to a FSMCommand instance.
1. DAQModules can be grouped by type (C++ class) or by Id (module instance reference)
1. ActionPlan has a "execution_policy" attribute which sets whether the modules referenced by each step should execute the command in parallel or in series. (Steps are always executed in series, but within each step, modules can receive the command in parallel or again in series.)

ActionPlans are validated by the application to ensure that every module type has registered methods corresponding to the command linked to the ActionPlan, and that only one ActionPlan is linked to the application for a given command. Most DUNE-DAQ applications are SmartDaqApplications, which may generate module instances using predefined rules. This can complicate the usage of DaqModulesGroupById and this mode is currently not allowed for SmartDaqApplications. Note that FSMCommand objects are usually defined by the CCM and included in a fsm.data.xml OKS database.

### Example test/config/appfwk.data.xml

The DAQModuleManager_test unit test defines several ActionPlans used within the test. For example, the "do_stuff" action:

```XML

<obj class="FSMCommand" id="stuff">
 <attr name="cmd" type="string" val="stuff"/>
 <attr name="optional" type="bool" val="0"/>
</obj>

<obj class="ActionPlan" id="stuff">
  <rel name="fsm" class="FSMCommand" id="stuff"/>
 <rel name="steps">
  <ref class="DaqModulesGroupByType" id="dummymodules_type_group"/>
 </rel>
</obj>

<obj class="DaqModulesGroupByType" id="dummymodules_type_group">
  <attr name="modules" type="class">
    <data val="DummyModule"/>
  </attr>
</obj>

```

Here, the FSMCommand is defined in the file because it is a non-standard command used for the purposes of this unit test. Normally, the FSMCommand instance would not be defined in the same location as the ActionPlan that uses it. The Action plan defines a single step where all modules in the "dummymodules_type_group" receive the "stuff" command in parallel (since parallel step execution is the default behavior when execution_policy is not specified). "dummymodules_type_group" groups all modules of type DummyModule.

Generally, FSMCommands are defined by the CCM group and the basic commands can be found in [fsm.data.xml](https://github.com/DUNE-DAQ/daqsystemtest/blob/develop/config/daqsystemtest/fsm.data.xml), but ActionPlan instances and DaqModulesGroupByType/ID instances are user-defined (examples are in moduleconfs.data.xml, described below).

The user-defined ActionPlans (such as "stuff" above) are associated with the Application instance as follows:

```XML
<obj class="DaqApplication" id="TestApp">
 <attr name="application_name" type="string" val="daq_application"/>
 <rel name="runs_on" class="VirtualHost" id="vlocalhost"/>
 <rel name="modules">
  <ref class="DummyModule" id="dummy_module_0"/>
 </rel>
 <rel name="action_plans">
  <ref class="ActionPlan" id="stuff"/> <!-- The stuff ActionPlan from above -->
  <ref class="ActionPlan" id="bad_stuff"/> <!-- Another ActionPlan this Application can execute -->
 </rel>
</obj>
```

## Notes

* DAQModules register their action methods with the DAQModuleManager, and this information is used in validation of the ActionPlans (e.g. that every DAQModule that has registered a command is called by the corresponding ActionPlan).
* ActionPlans refer to FSMCommand objects as defined by the CCM. New FSMCommands may be added, but should be integrated into the state machine in consultation with CCM experts.
* Within each step of an ActionPlan, whether executing in series or in parallel, the modules will be called in the order in which they are declared to the Application. In series mode, the future has `wait()` called for each module in the step, and in parallel mode, the futures are all started and the results are collected by a loop which calls `wait()` on each individually.

## Further Examples

### https://github.com/DUNE-DAQ/daqsystemtest/blob/develop/config/daqsystemtest/moduleconfs.data.xml#L134

```XML
<obj class="ActionPlan" id="readout-start">
 <attr name="execution_policy" type="enum" val="modules-in-parallel"/>
 <rel name="command" class="FSMCommand" id="start"/>
 <rel name="steps">
  <ref class="DaqModulesGroupByType" id="aggregator-step"/>
  <ref class="DaqModulesGroupByType" id="tp-handler-step"/>
  <ref class="DaqModulesGroupByType" id="dlh-step"/>
  <ref class="DaqModulesGroupByType" id="data-source-step"/>
 </rel>
</obj>
```

This ActionPlan consists of four steps, with each step sending the command to the modules matched by the group in parallel. Therefore:
1. the FragmentAggregator will run first (there is only one FA per app),
1. when the FA is complete, all of the TP Handlers will receive "start" in parallel
1. once they are all complete, then the DataLinkHandlers
1. finally the FDFakeReaderModules

### https://github.com/DUNE-DAQ/daqsystemtest/blob/develop/config/daqsystemtest/moduleconfs.data.xml#L199

```XML
<obj class="ActionPlan" id="tc-maker-start">
 <attr name="execution_policy" type="enum" val="modules-in-series"/>
 <rel name="command" class="FSMCommand" id="start"/>
 <rel name="steps">
  <ref class="DaqModulesGroupByType" id="ta-handler-step"/>
  <ref class="DaqModulesGroupByType" id="subscriber-step"/>
 </rel>
</obj>
```

This ActionPlan, by contrast, uses the modules-in-series execution policy, so for an application with "ta-handler-01", "ta-handler-02", and "ta-subscriber-01", the ActionPlan will result in:
1. (Step 1A) "ta-handler-01" executing "start"
1. (Step 1B) once that is complete, "ta-handler-02" will execute "start"
1. (Step 2) finally "ta-subscriber-01" will receive "start"
