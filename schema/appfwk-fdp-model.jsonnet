// This is a "model" file specific for the "cmd" schema of the appfwk.
//
// It's job is to provide the glue between the schema data structure
// and the structure that the templates expect.  It "bakes" in some
// information that might otherwise be provided by TLAs or grapfts.
// Other schema have their own models which will look almost identical
// to this one.  We "bake" in this repetition in these schema-specific
// model files in order to simplify what gets exposed to CMake.
//
// To customize to a new schema likely one just needs to find the
// short schema name and replace it in hte import and the "path".
// Because Jsonent does not allow "computed imports" we can not 
// parameterize this much shorter.

local model = import "appfwk-model.jsonnet";
model(import "appfwk-fdp-schema.jsonnet",
      "dunedaq.appfwk.fdp", ["dunedaq"])
