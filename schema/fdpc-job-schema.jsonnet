// This is a "command" schema describing an appfwk process that
// consists of a fdp and a fdc.  It's intended NOT for codegen but for
// construction and validation of an entire sequence of command
// objects.  As such, there are no opaque types (ie, no "any").
//
// The idea is to define one (eventually among many) "classes of
// application", which specify which modules are in the appfwk app and
// thus may fully specify the schema of this class's command objects.

local moo = import "moo.jsonnet";

local all_hiers = [moo.oschema.hier(import "appfwk-cmd-schema.jsonnet"),
                   moo.oschema.hier(import "appfwk-fdp-schema.jsonnet"),
                   moo.oschema.hier(import "appfwk-fdc-schema.jsonnet")];
// appfwk schema
local afs = std.foldl(std.mergePatch, all_hiers, {}).dunedaq.appfwk;

local replace_field(r, f) = r {
    fields:[of for of in r.fields if of.name != f.name] + [f] };


local ccm = moo.oschema.schema("dunedaq.ccm");

// appfwk related
local af = {

    idnames: ["undef","init","conf","start","stop","scrap","fini"],

    id: ccm.enum("Id", self.idnames, default="undef",
                 doc="RC command identifiers"),

    ini: ccm.record("InitCmd", [
        ccm.field("id", self.id,
                  doc="fixme: this enum should be moved to appfwk.cmd"),
        ccm.field("data", afs.cmd.Init),
    ]),

    cmd: ccm.record("AnyCmd", [
        ccm.field("id", self.id,
                  doc="fixme: this enum should be moved to appfwk.cmd"),                  
        ccm.field("data", afs.cmd.CmdObj),
    ]),
    
};                      


af




