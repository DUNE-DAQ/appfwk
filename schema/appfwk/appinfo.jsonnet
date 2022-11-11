// This is the application info schema used by the appfwk application.
// It describes the information object structure passed by the application 
// for operational monitoring

local moo = import "moo.jsonnet";
local s = moo.oschema.schema("dunedaq.appfwk.appinfo");

local info = {
   state_t : s.string("state_t", moo.re.ident,
                  doc="A string field"), 
   busy_t : s.boolean("busy_t",
                  doc="Busy flag"), 
   err_t : s.boolean("error_t",
                  doc="Error flag"),

   info: s.record("Info", [
       s.field("state", self.state_t, doc="State"),
       s.field("host",  self.state_t, doc="hostname of the application"),
       s.field("busy", self.busy_t, 0,  doc="Busy flag"), 
       s.field("error", self.err_t, 0, doc="Error flag")
   ], doc="General application information")
};

moo.oschema.sort_select(info) 
