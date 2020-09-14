// This is the "application" level schema ("app") used by appfwk
// applications to handle framework level command data objects.
//
// It governs the layer of command object strcture between the top
// command delivery layer ("cmd" schema) and the layer specific to
// individual instances of DAQModule.
// 
// It describes the information required for appfwk to construct
// queues and modules and to implement a "command dispatch protocol"
// which dispatches per-module instance command information.

local moo = import "moo.jsonnet";

// A schema builder in the given path (namespace)
local ns = "dunedaq.appfwk.app";
local s = moo.oschema.schema(ns);

// A temporary structure in which to constrct the types of the schema.
local as = {
    data: s.any("Data",
                doc="An opaque object holding lower layer substructure"),
    
    kindname: s.string("KindName", moo.re.ident_only,
                       doc="Name of a kind (class/type) of target instance"),
    instname: s.string("InstName", moo.re.ident_only,
                       doc="Name of a target instance of a kind"),
    kindinst: s.record("KindInst", [
        s.field("kind", self.kindname, doc="A kind (class/type) name"),
        s.field("inst", self.kindname, doc="An instance name"),
    ], doc="Associate a kind and an instance name"),
    kindinsts: s.sequence("KindInsts", self.kindinst,
                          doc="A sequence of KindInst"),

    addrdat: s.record("AddrDat", [
        s.field("ki", self.kindinst, doc="The addressed instance"),
        s.field("data", self.data, doc="The data object for the instance"),
    ], doc="Address a data object to a kind of instance"),
    addrdats: s.sequence("AddrDats", self.addrdat,
                         doc="A sequence of AddrDat"),

    addressed: s.record("Addressed", [
        s.field("addrdats", self.addrdats,
                doc="The data objects addressed to kinds of instances"),
    ], doc="The app level data object"),

};

// FIXME: what follows really should be a separate schema so that the
// above remains general-purpose to non-appfwk apps.  However, that's
// not currently allowed.  We'll keep a locally logical separation but
// merge the whole mess on output.  Just take care not to have any
// keys colide.

// Queue-specific schema
local qs = {

    // A sub context to collect schema related to queues
    capacity: s.number("QueueCapacity", dtype="u8",
                       doc="Capacity of a queue"),
                           
    kind: s.enum("QueueKind",
                 ["Unknown", "StdDeQueue", "FollySPSCQueue", "FollyMPMCQueue"],
                 doc="The kinds (types/classes) of queues"),

    qinit: s.record("QueueInit", [
        s.field("kind", self.kind, doc="The kind of queue"),
        s.field("capacity", self.capacity, doc="The queue capacity"),
    ], doc="Queue initialization data structure"),

};

// DAQModule-sepecific (base class level) schema
local ms = {
    pin: s.string("PluginName", moo.re.ident_only,
                  doc="Name of a plugin"),

    minit: s.record("ModInit", [
        s.field("plugin", self.pin, doc="Module plugin name"),
        s.field("data", as.data, doc="Instance specific data"),
    ], doc="Module initialization data structure"),

};

// Output a topologically sorted array for all types in our namespace.
moo.oschema.sort_select(as + qs + ms, ns)
