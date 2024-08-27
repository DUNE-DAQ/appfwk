# Action Plans

## Overview

An ActionPlan defines a series of ActionSteps, each one consisting of a set of Action objects. The Action object provides "targeting" information in the form of module class and id, with the default id of `"*"` matching all modules of the given class. Each ActionPlan is associated with a FSMCommand object, and is run by the appliction when it recieves the corresponding command. If a command is received and no ActionPlan is defined, the application currently runs a "dummy" ActionPlan consisting of a single step where registered Actions matching the command name are all run in parallel.

## Defining an ActionPlan

ActionPlans are defined in configuration using these objects:

```XML
 <class name="ActionPlan" description="A set of parallel steps for an application to carry out a command">
   <relationship name="fsm" class-type="FSMCommand" low-cc="one" high-cc="one" is-composite="no" is-exclusive="no" is-dependent="no"/>
  <relationship name="steps" class-type="ActionStep" low-cc="zero" high-cc="many" is-composite="no" is-exclusive="no" is-dependent="no" ordered="yes"/>
 </class>

 <class name="ActionStep">
  <relationship name="actions" class-type="Action" low-cc="zero" high-cc="many" is-composite="no" is-exclusive="no" is-dependent="no"/>
 </class>

 <class name="Action">
   <attribute name="module_class" type="class" is-not-null="yes" init-value="DaqModule"/>
   <attribute name="module_id" type="string" is-not-null="yes" init-value="*"/>
 </class>
```

1. ActionPlan relates a set of ActionSteps to a FSMCommand instance.
1. ActionStep lists the Actions to run in parallel at a given point in the sequence
1. 1. Action targets a specific instance of a module class, or all modules of that class using module_id `"*"`

ActionPlans are validated by the application to ensure that every module type has registered methods corresponding to the command linked to the ActionPlan, and that only one ActionPlan is linked to the application for a given command.

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
  <ref class="ActionStep" id="run_on_all_dummymodules_step"/>
 </rel>
</obj>

<obj class="ActionStep" id="run_on_all_dummymodules_step">
  <rel name="actions">
    <ref class="Action" id="all_DummyModules_action"/>
  </rel>
</obj>

<obj class="Action" id="all_DummyModules_action">
  <attr name="module_class" type="class" val="DummyModule"/>
  <attr name="module_id" type="string" val="*"/>
</obj>

```

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
