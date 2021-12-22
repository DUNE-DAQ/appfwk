// This is the "command" schema ("cmd") used by appfwk applications.
//
// It governs the expected top-level structure of command objects.
//
// Its objects hold substructure as an "any" type which is expected to
// be interpreted by the next lower layer such as described in the
// appfwk "application" schema ("app").

local moo = import "moo.jsonnet";
local s = moo.oschema.schema("dunedaq.appfwk.app");
local s_rccmd = import "rcif/cmd.jsonnet";
local s_nwmgr = import "networkmanager/nwmgr.jsonnet";

local nwmgr = moo.oschema.hier(s_nwmgr).dunedaq.networkmanager.nwmgr;
local rccmd = moo.oschema.hier(s_rccmd).dunedaq.rcif.cmd;
local cmd = moo.oschema.hier(s_rccmd).dunedaq.cmdlib.cmd;

// A temporary schema construction context.
local cs = {

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
        s.field("kind", self.qkind,
                doc="The kind (type) of queue"),
        s.field("inst", self.inst,
                doc="Instance name"),
        s.field("capacity", self.capacity,
                doc="The queue capacity"),
    ], doc="Queue specification"),

    qspecs: s.sequence("QueueSpecs", self.qspec,
                       doc="A sequence of QueueSpec"),

    mspec: s.record("ModSpec", [
        s.field("plugin", self.plugin,
                doc="Name of a plugin providing the module"),
        s.field("inst", self.inst,
                doc="Module instance name"),
        s.field("data", cmd.Data, optional=true,
                doc="Specific to the module implementation"),
    ], doc="Module specification"),

    mspecs: s.sequence("ModSpecs", self.mspec,
                       doc="A sequence of ModSpec"),

    // fixme: probably should be an enum: ["unknown","input","output"]
    dir: s.string("QueueDir", pattern="(^input$)|(^output$)",
                  doc="Direction of queue from the point of view of a module holding an endpoint"),

    qinfo: s.record("QueueInfo", [
        s.field("inst", self.inst,
                doc="The queue instance name"),
        s.field("name", self.label,
                doc="The name by which this queue is known to the module"),
        s.field("dir", self.dir,
                doc="The direction of the queue as seen by the module"),
    ], doc="Information for a module to find a Queue"),
    qinfos: s.sequence("QueueInfos", self.qinfo,
                       doc="A sequence of QueueInfo"),

    modinit: s.record("ModInit", [
        s.field("qinfos", self.qinfos,
                doc="Information for a module to find its queue"),
    ], doc="A standardized portion of every ModSpec.data"),
        
    init: s.record("Init", [
        s.field("queues", self.qspecs, optional=true,
                doc="Initial Queue specifications"),
        s.field("modules", self.mspecs,
                doc="Initial Module specifications"),
        s.field("nwconnections", nwmgr.Connections, optional=true,
                doc="Initial NW connections specifications"),
    ], doc="The app-level init command data object struction"),

};

// Output a topologically sorted array.
s_nwmgr + s_rccmd + moo.oschema.sort_select(cs)
