// This is the "command" schema ("cmd") used by appfwk applications.
//
// It governs the expected top-level structure of command objects.
//
// Its objects hold substructure as an "any" type which is expected to
// be interpreted by the next lower layer such as described in the
// appfwk "application" schema ("app").

local moo = import "moo.jsonnet";
local s = moo.oschema.schema("dunedaq.appfwk.cmd");
local s_rccmd = import "rcif/cmd.jsonnet";
local rccmd = moo.oschema.hier(s_rccmd).dunedaq.rcif.cmd;
local cmd = moo.oschema.hier(s_rccmd).dunedaq.cmdlib.cmd;

// A temporary schema construction context.
local cs = {

    // fixme: specify a pattern that itself matches any regex?
    match: s.string("Match", doc="String used as a regex match"),

    mcmd: s.record("AddressedCmd", [
        s.field("match", self.match,
                doc="A regex that matches on module instance names"),
        s.field("data", cmd.Data, optional=true,
                doc="The module-level command data object"),
    ], doc="General, non-init module-level command data structure"),
    mcmds: s.sequence("AddressedCmds", self.mcmd,
                     doc="A sequence of AddressedCmd"),

    mcmdobj: s.record("CmdObj", [
        s.field("modules", self.mcmds,
                doc="Addressed, module command objects"),
    ], doc="Structure of app-level, non-init command object"), 

};

// Output a topologically sorted array.
s_rccmd + moo.oschema.sort_select(cs)
