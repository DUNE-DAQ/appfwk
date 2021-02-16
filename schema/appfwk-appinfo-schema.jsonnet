// This is the application info schema used by the appfwk application.
// It describes the information object structure passed by the application 
// for operational monitoring

local moo = import "moo.jsonnet";
local s = moo.oschema.schema("dunedaq.appfwk.info");

local info = {
   partition : s.string("Partition name", moo.re.ident,
                  doc="A string field"), 
   application : s.string("Application name", moo.re.ident,
                  doc="A string field"), 
   state : s.string("State name", moo.re.ident,
                  doc="A string field"), 
   busy : s.bool("Busy state",
                  doc="Busy flag"), 
   error : s.bool("Error state",
                  doc="Error flag"),

   info: s.record("Info", [
       s.field("partition_name", self.partition, doc="Partition name"), 
       s.field("app_name", self.application, doc="Application name"), 
       s.field("state", self.state, doc="State"), 
       s.field("busy", self.busy, doc="Busy flag"), 
       s.field("error", self.error, doc="Error flag")
   ], doc="General application information")
};

moo.oschema.sort_select(info, s) 
