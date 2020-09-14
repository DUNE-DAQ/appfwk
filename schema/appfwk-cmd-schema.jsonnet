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

    // FIXME: this should be an enum but I'm not allowed to do so yet.
    cmdid: s.string("Name", pattern=moo.re.ident_only,
                    doc="The command name.  FIXME: this should be an enum!"),
    
    command: s.record("Command", [
        s.field("id", self.cmdid, doc="Identify the type of command"),
        s.field("data", self.data, doc="Command data object with type-specific structure"),
    ], doc="Top-level command object structure"),
};

// Output a topologically sorted array.
moo.oschema.sort_select(cs)
