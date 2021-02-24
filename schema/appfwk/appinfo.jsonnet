// This is the application info schema used by the appfwk application.
// It describes the information object structure passed by the application 
// for operational monitoring

local moo = import "moo.jsonnet";
local s = moo.oschema.schema("dunedaq.appfwk.appinfo");

local info = {
   cl : s.string("class_s", moo.re.ident,
                  doc="A string field"), 
//   partition : s.string("partition", moo.re.ident,
//                  doc="A string field"), 
//   application : s.string("application", moo.re.ident,
//                  doc="A string field"), 
   state : s.string("state_v", moo.re.ident,
                  doc="A string field"), 
   busy : s.boolean("busy_v",
                  doc="Busy flag"), 
   err : s.boolean("error_v",
                  doc="Error flag"),

   info: s.record("Info", [
       s.field("class_name", self.cl, "appinfo", doc="Info class name"),
//       s.field("partition_name", self.partition, doc="Partition name"), 
//       s.field("app_name", self.application, doc="Application name"), 
       s.field("state", self.state, doc="State"), 
       s.field("busy", self.busy, 0,  doc="Busy flag"), 
       s.field("error", self.err, 0, doc="Error flag")
   ], doc="General application information")
};

moo.oschema.sort_select(info) 
