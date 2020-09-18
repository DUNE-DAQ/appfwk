local moo = import "moo.jsonnet";

local cmd = import "appfwk-cmd-make.jsonnet";
local fdp = import "appfwk-fdp-make.jsonnet";
local fdc = import "appfwk-fdc-make.jsonnet";

local qname = "hose";            // the name of the single queue in this job


[

    cmd.init([cmd.qspec("hose", "StdDeQueue", 10)],
             [cmd.mspec("fdp", "FakeDataProducerDAQModule",
                        cmd.qinfo(fdp.queue, qname, cmd.qdir.output)),
              cmd.mspec("fdc", "FakeDataConsumerDAQModule",
                        cmd.qinfo(fdc.queue, qname, cmd.qdir.input))]),


    cmd.conf([cmd.mcmd("fdp", fdp.conf(10,-4,14)),
              cmd.mcmd("fdc", fdc.conf(10,-4,14))]),
    
    // send by match-all
    cmd.start(42),

    // send to modules in explicit order
    cmd.stop([cmd.mcmd("fdp"), cmd.mcmd("fdc")]),

    // cmd.scrap(),

    // cmd.fini(),

]

