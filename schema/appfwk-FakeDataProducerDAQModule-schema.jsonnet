// The schema used by classes in the appfwk code tests.
//
// It is an example of the lowest layer schema below that of the "cmd"
// and "app" and which defines the final command object structure as
// consumed by instances of specific DAQModule implementations (ie,
// the test/Fake* modules).

local moo = import "moo.jsonnet";

// A schema builder in the given path (namespace)
local ns = "dunedaq.appfwk.fakedataproducerdaqmodule";
local s = moo.oschema.schema(ns);

// Object structure used by the test/fake producer module
local fakedataproducerdaqmodule = {
    size: s.number("Size", "u8",
                   doc="A count of very many things"),

    count : s.number("Count", "i4",
                     doc="A count of not too many things"),

    conf: s.record("Conf", [
        s.field("nIntsPerVector", self.size, 10,
                doc="Number of numbers"),
        s.field("starting_int", self.count, -4,
                doc="Number to start with"),
        s.field("ending_int", self.count, 14,
                doc="Number to end with"),
        s.field("queue_timeout_ms", self.count, 100,
                doc="Milliseconds to wait on queue before timing out"),
        s.field("wait_between_sends_ms", self.count, 1000,
                doc="Millisecs to wait between sending"),
    ], doc="Fake Data Producer DAQ Module Configuration"),

};

moo.oschema.sort_select(fakedataproducerdaqmodule, ns)

