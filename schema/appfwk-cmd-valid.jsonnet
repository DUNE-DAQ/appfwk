// Emit an augmentation of the basic appfwk-cmd-schema to include
// schema that covers validating specific commands.  This schema does
// not necessarily map well to generating code but is intstead
// intended to be derived to, eg, JSON Schema for validating a command
// object.

// FIXME: we MUST gives a JSON Schema ID which should be a URL that
// points to the content we are generating.  We do not yet have our
// ducks in a row to actually answer this URL.  So, for now we just
// punt.
local json_schema_id = "https://dune-daq.github.io/appfwk/schema/appfwk-cmd-valid.json";

local moo = import "moo.jsonnet";
local s = moo.oschema.schema("dunedaq.appfwk.cmd");
local cmd = moo.oschema.hier(import "appfwk-cmd-schema.jsonnet").dunedaq.appfwk.cmd;

local cs = {
    init_id: s.string("InitId", pattern="^init$"),
    init_data: s.record("InitData", [
        s.field("queues", cmd.QueueSpecs), s.field("modules", cmd.ModSpecs)]),
    init: s.record("InitCommand", [
        s.field("id", self.init_id,
                doc="The init command ID is init"),
        s.field("data", self.init_data,
                doc="Init command holds specs on queues and modules"),
    ], doc="An appfwk init command"),

    conf_id: s.string("ConfId", pattern="^conf$"),
    conf_data: s.record("ConfData",[ s.field("modules", cmd.AddressedCmds) ]),
    conf: s.record("ConfCommand", [
        s.field("id", self.conf_id,
                doc="The conf command ID is conf"),
        s.field("data", self.conf_data,
                doc="Conf command holds module confs"),
    ], doc="An appfwk conf command"),
        

    // fixme: start, stop, scrap, fini

    command: s.anyOf("AppfwkCommand",[ self.init, self.conf ]),
};
local types = moo.oschema.sort_select(cs);
//types
moo.jschema.convert(types, json_schema_id)


