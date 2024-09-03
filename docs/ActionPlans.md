# Action Plans

## Overview

An ActionPlan defines a series of steps consisting of groups of modules, which are executed in response to a command from CCM. Groups of modules are defined either by module class or by module instances, and the execution of each step is in parallel by default, but can be changed to serial execution if needed. Each ActionPlan is associated with a FSMCommand object, and is run by the appliction when it recieves the corresponding command. If a command is received and no ActionPlan is defined, the application currently runs a "dummy" ActionPlan consisting of a single step where modules with the command registered are all run in parallel.

Action Plans allow for much finer-grained control over the execution of a command within an application, allowing for modules that have dependencies on one another to execute their commands correctly. It also introduces parallelization of command execution within each step, which helps with certain time-consuming module commands (e.g. configuring hardware on a link). The current implmentation uses std::future objects and a catch-all threading pattern to ensure that errors executing steps within an action plan do not lead to program crashes.

## Defining an ActionPlan

ActionPlans are defined in configuration using these objects:

```XML
 <class name="ActionPlan" description="A set of parallel steps for an application to carry out a command">
  <attribute name="execution_policy" type="enum" description="How the application should execute steps of the action plan" range="modules-in-parallel,modules-in-series" init-value="modules-in-parallel"/>
  <relationship name="command" class-type="FSMCommand" low-cc="one" high-cc="one" is-composite="no" is-exclusive="no" is-dependent="no"/>
  <relationship name="steps" class-type="DaqModulesGroup" low-cc="zero" high-cc="many" is-composite="no" is-exclusive="no" is-dependent="no"/>
 </class>

 <class name="DaqModulesGroup" description="Represents a group of DAQ Modules that can run commands in parallel as one of the steps of an ActionPlan." is-abstract="yes">
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
1. ActionPlan has a "execution_policy" attribute which sets whether the groups should execute the command in parallel or in series.

ActionPlans are validated by the application to ensure that every module type has registered methods corresponding to the command linked to the ActionPlan, and that only one ActionPlan is linked to the application for a given command. Also, SmartDaqApplications are only allowed to use ActionPlans which group modules by type, so the application validates that all ActionPlan steps are of type DaqModulesGroupByType in that case. Note that FSMCommand objects are usually defined by the CCM and included in a fsm.data.xml OKS database.

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

Here, the FSMCommand is defined in the file because it is a non-standard command used for the purposes of this unit test. Normally, the FSMCommand instance would not be defined in the same location as the ActionPlan that uses it.

The ActionPlans are associated with the Application instance as follows:

```XML
<obj class="DaqApplication" id="TestApp">
 <attr name="application_name" type="string" val="daq_application"/>
 <rel name="runs_on" class="VirtualHost" id="vlocalhost"/>
 <rel name="modules">
  <ref class="DummyModule" id="dummy_module_0"/>
 </rel>
 <rel name="action_plans">
  <ref class="ActionPlan" id="stuff"/>
  <ref class="ActionPlan" id="bad_stuff"/>
 </rel>
</obj>
```

## Notes

* DAQModules register their action methods in the same way as before, however the specification of valid states for an action has been removed
* ActionPlans refer to FSMCommand objects as defined by the CCM. New FSMCommands may be added, but should be integrated into the state machine in consultation with CCM experts.
