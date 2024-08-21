# Action Plans

## Overview

An ActionPlan defines a series of ActionSteps, each one consisting of a number of parallel Actions. An Action is a method registered by a DAQModule with a specific name. Each ActionPlan is associated with a FSMCommand object, and is run by the appliction when it recieves the corresponding command. If a command is received and no ActionPlan is defined, the application currently runs a "dummy" ActionPlan consisting of a single step where registered Actions matching the command name are all run in parallel.

## Defining an ActionPlan

ActionPlans are defined in configuration using three simple objects:

```XML
 <class name="ActionPlan" description="A set of parallel steps for an application to carry out a command">
   <relationship name="fsm" class-type="FSMCommand" low-cc="one" high-cc="one" is-composite="no" is-exclusive="no" is-dependent="no"/>
  <relationship name="steps" class-type="ActionStep" low-cc="zero" high-cc="many" is-composite="no" is-exclusive="no" is-dependent="no" ordered="yes"/>
 </class>

 <class name="ActionStep">
  <relationship name="actions" class-type="Action" low-cc="one" high-cc="many" is-composite="no" is-exclusive="no" is-dependent="no"/>
 </class>

 <class name="Action">
  <attribute name="method_name" type="string" is-not-null="yes"/>
   <attribute name="module" type="class" is-not-null="yes" init-value="DaqModule"/>
 </class>
```

1. ActionPlan relates a set of ActionSteps to a FSMCommand instance.
1. ActionStep is a set of parallel Action instances to run
1. Action defines the method name and module type to run

Actions are validated by the application to ensure that every module type has registered methods corresponding to the ActionPlan Actions associated with the application.

### Example test/config/appfwk.data.xml

The DAQModuleManager_test unit test defines several ActionPlans used within the test. For example, the "do_stuff" action:

```XML

<obj class="FSMCommand" id="stuff">
 <attr name="cmd" type="string" val="stuff"/>
 <attr name="optional" type="bool" val="0"/>
</obj>

<obj class="Action" id="dummy_module_0_stuff">
 <attr name="method_name" type="string" val="stuff"/>
 <attr name="module" type="class" val="DummyModule"/>
</obj>

<obj class="ActionPlan" id="stuff">
  <rel name="fsm" class="FSMCommand" id="stuff"/>
 <rel name="steps">
  <ref class="ActionStep" id="stuff_step_1"/>
 </rel>
</obj>

<obj class="ActionStep" id="stuff_step_1">
 <rel name="actions">
  <ref class="Action" id="dummy_module_0_stuff"/>
 </rel>
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
* Actions target module types, with the assumption that if multiple modules of the same class are present within an application, they will all run their defined Actions in unison (or at least within the same parallel processing step)
* This system allows for multiple Actions to run from a single module type in response to a single command. Developers may take advantage of this to sequence actions within their modules in cases where dependencies exist within a single application for the results of these actions. (e.g. Modules A and B do "pre-conf", Module A then uses a connection to Module B set up in "pre-conf" in "post-conf")