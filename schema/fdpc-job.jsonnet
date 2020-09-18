local moo = import "moo.jsonnet";

local cmd = import "appfwk-cmd-make.jsonnet";
local fdp = import "appfwk-fdp-make.jsonnet";
local fdc = import "appfwk-fdc-make.jsonnet";

[
    cmd.init([cmd.qspec("hose", "StdDeQueue", 10)],
              [cmd.mspec("fdp", "FakeDataProducerDAQModule"),
               cmd.mspec("fdc", "FakeDataConsumerDAQModule")]),


    cmd.conf([cmd.mcmd("fdp", fdp.conf(10,-4,10)),
              cmd.mcmd("fdc", fdc.conf(10,-4,10))]),

    cmd.start(42),

    cmd.stop(),

    cmd.scrap(),

    cmd.fini(),

]

