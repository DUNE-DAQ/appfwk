// This is the queue info schema used by the queue application.
// It describes the information object structure passed by the application 
// for operational monitoring

local moo = import "moo.jsonnet";
local s = moo.oschema.schema("dunedaq.appfwk.queueinfo");

local info = {
   cl : s.string("class_s", moo.re.ident,
                  doc="A string field"), 

   uint8  : s.number("uint8", "u8",
                     doc="An unsigned of 8 bytes used for counters"),

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
       s.field("class_name", self.cl, "queueinfo", doc="Info class name"),
       s.field("capacity",   self.unit8, 0, doc "Maximum queue capacity" ),
       s.field("number_of_elements", self.unit8, 0, doc "Elements in the queue" )
   ], doc="General Queue information")
};

moo.oschema.sort_select(info) 
