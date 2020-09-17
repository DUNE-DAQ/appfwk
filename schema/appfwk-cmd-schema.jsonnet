// This is the "command" schema ("cmd") used by appfwk applications.
//
// It governs the expected top-level structure of command objects.
//
// Its objects hold substructure as an "any" type which is expected to
// be interpreted by the next lower layer such as described in the
// appfwk "application" schema ("app").

local moo = import "moo.jsonnet";
local s = moo.oschema.schema("dunedaq.appfwk.cmd");

// A temporary schema construction context.
local cs = {
    data: s.any("Data",
                doc="An opaque object holding lower layer substructure"),

    // fixme: this should be an enum but I'm not allowed to make it so yet.
    cmdid: s.string("CmdId", pattern=moo.re.ident_only,
                    doc="The command name.  FIXME: this should be an enum!"),
    // cmdid: s.enum("CmdId", ["unknown","init","conf","start","stop","scrap","fini"],
    //                 "unknown", doc="The known command types"),
    
    command: s.record("Command", [
        s.field("id", self.cmdid, doc="Identify the type of command"),
        s.field("data", self.data, doc="Command data object with type-specific structure"),
    ], doc="Top-level command object structure"),

    plugin: s.string("PluginName", moo.re.ident_only,
                     doc="Name of a plugin"),
    inst: s.string("InstName", moo.re.ident_only,
                   doc="Name of a target instance of a kind"),
    label: s.string("Label", moo.re.ident_only,
                   doc="A label hard-wired into code"),
    qkind: s.enum("QueueKind",
                  ["Unknown", "StdDeQueue", "FollySPSCQueue", "FollyMPMCQueue"],
                  doc="The kinds (types/classes) of queues"),
    capacity: s.number("QueueCapacity", dtype="u8",
                       doc="Capacity of a queue"),
                           
    qspec: s.record("QueueSpec", [
        s.field("kind", self.qkind, doc="The kind (type) of queue"),
        s.field("inst", self.inst, doc="Instance name"),
        s.field("capacity", self.capacity, doc="The queue capacity"),
    ], doc="Queue specification"),
    qspecs: s.sequence("QueueSpecs", self.qspec, "A sequence of QueueSpec"),

    mspec: s.record("ModSpec", [
        s.field("plugin", self.plugin, doc="Name of a plugin providing the module"),
        s.field("inst", self.inst, doc="Module instance name"),
        s.field("data", self.data, doc="Specific to the module implementation"),
    ], doc="Module specification"),
    mspecs: s.sequence("ModSpecs", self.mspec, "A sequence of ModSpec"),

    // fixme: probably should be an enum: ["unknown","input","output"]
    dir: s.string("QueueDir", pattern="(^input$)|(^output$)",
                  doc="Direction of queue from the point of view of a module holding an endpoint"),

    qinfo: s.record("QueueInfo", [
        s.field("inst", self.inst, doc="The queue instance name"),
        s.field("name", self.label, doc="The name by which this queue is known to the module"),
        s.field("dir", self.dir, doc="The direction of the queue as seen by the module"),
    ], doc="Information for a module to find a Queue"),
    qinfos: s.sequence("QueueInfos", self.qinfo, doc="A sequence of QueueInfo"),

    modinit: s.record("ModInit", [
        s.field("qinfos", self.qinfos, "Information for a module to find its queue"),
    ], doc="A standardized portion of every ModSpec.data"),
        
    init: s.record("Init", [
        s.field("queues", self.qspecs, doc="Initial Queue specifications"),
        s.field("modules", self.mspecs, doc="Initial Module specifications"),
    ], doc="The app-level init command data object struction"),

    // fixme: specify a pattern that itself matches any regex?
    match: s.string("Match", doc="String used as a regex match"),

    mcmd: s.record("AddressedCmd", [
        s.field("match", self.match, doc="A regex that matches on module instance names"),
        s.field("data", self.data, doc="The module-level command data object"),
    ], doc="General, non-init module-level command data structure"),
    mcmds: s.sequence("AddressedCmds", self.mcmd,
                     doc="A sequence of AddressedCmd"),

    mcmdobj: s.record("CmdObj", [
        s.field("modules", self.mcmds, doc="Addressed, module command objects"),
    ], doc="Structure of app-level, non-init command object"), 

};

// Output a topologically sorted array.
moo.oschema.sort_select(cs)
