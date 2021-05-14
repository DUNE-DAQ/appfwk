// This is the queue info schema used by the queue application.
// It describes the information object structure passed by the application 
// for operational monitoring

local moo = import "moo.jsonnet";
local s = moo.oschema.schema("dunedaq.appfwk.queueinfo");

local info = {
   uint8  : s.number("uint8", "u8",
                     doc="An unsigned of 8 bytes used for counters"),

   info: s.record("Info", [
       s.field("capacity",   self.uint8, 0, doc="Maximum queue capacity" ),
       s.field("number_of_elements", self.uint8, 0, doc="Elements in the queue" )
   ], doc="General Queue information")
};

moo.oschema.sort_select(info) 
