Trigger Candidate Generator in ProtoDUNE-I
===========================================

https://cdcvs.fnal.gov/redmine/projects/dune-artdaq/repository/revisions/develop/entry/dune-artdaq/Generators/Candidate_generator.cc


One `TPWindow` per input link (ie 10 per APA)
`TPWindow`s connected to a `TPZipper`
`TPZipper` connected to `TPFilter`, which runs the actual selection algorithm
`TPFilter` emits trigger candidates, which go to MLT via ptmp (ie zeromq), and are supposed to also go to getNext_() so the BR proper can get them in response to triggers

```
 from hit finder --> TPWindow --|                              |--> to MLT
 from hit finder --> TPWindow --|--> TPZipper --> TPFilter --> |
 from hit finder --> TPWindow --|                              |--> to getNext()
 ...                         ...|
```

All sockets were PUB/SUB or PUSH/PULL depending on required drop/block semantics.  Typically PUB/SUB is used in a layer that uses `tcp://` transport and PUSH/PULL is used for layers using `inproc://`.  An fanout requires PUB as PUSH uses round-robin routing.

Possible user modules:
----------------------

`TPWindow`: read in a stream of TPSets and output a new stream of TPSets whose timespans and boundaries are set by the config, with a (soft) maximum latency guarantee

`TPZipper`: read in TPSets from multiple `TPWindow` instances, aggregate them and output in strict time order, with a (soft) maximum latency guarantee

`TPFilter`: filter TPSets using an algorithm specified by config (and loaded via factory pattern). Here, the algorithm is the trigger candidate finder. Output TPSets encode the trigger candidate in a slightly hacky way.

`TPQuery`: buffer TPSets and respond to query with all available TPSets based on a given time range.  This kind of lookup requires *interval ranges* and is implemented in https://github.com/brettviren/tpquery.

Requirements on communication channels
--------------------------------------

Input rates in ProtoDUNE are ~100 kHz of individual hits (TPs) per link into each `TPWindow`.  Bundling recent TPs into TPSets can reduce message rate.  8kHz/link was typical max.  Needs for applying windowing downstream may change this max.

`TPWindow` and `TPZipper` provide timing guarantees, so we need "poll with timeout" on sources of data.

`TPZipper` aggregates across multiple sources. Would like nice ways to do this. It's done via poll() in the `TPZipper` implementation.

The output from `TPFilter` needs to be duplicated to MLT (on another machine) and to a buffer somewhere (probably the same process) for later readout if triggered.

