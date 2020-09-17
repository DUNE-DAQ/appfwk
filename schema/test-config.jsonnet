[
    {
        id: "init",
        data: {                 // "any" / app::Init struct
            queues: [           // sequence of ...
                {               // ... app::QueueSpec
                    kind: "StdDeQueue", // enum
                    inst: "hose",       // string(re.ident)
                    capacity: 10,       // number(i4,>0)
                },
            ],
            modules: [          // sequence of...
                {               // app::ModSpec
                    plugin: "FakeDataProducerDAQModule", // string(re.ident)
                    inst: "fdp",  // string(re.ident)
                    data: {       // "any" / app::ModInit
                        qinfos: [             // sequence of...
                            {                 // app::QueueInfo
                                inst: "hose", // string(re.ident)
                                name: "output", // string(re.indent), known to mod C++
                                dir: "output",
                            }, // ... possibly more
                        ],
                        // may extend app::ModInit attributes
                        // may add fdp::Init attributes
                    },
                },
                {               // etc for a consumer
                    plugin: "FakeDataConsumerDAQModule",
                    inst: "fdc",
                    data: {
                        qinfos: [
                            {
                                inst: "hose",
                                name: "input",
                                dir: "input",
                            },
                        ],
                    },
                }
            ],                  // end of module init data
        },                      // end of init command data
    },                          // end of init command

    {
        id: "conf",
        data: {
            modules: [          // sequence of ...
                {               // ... app::ModConf 
                    inst: "fdp",
                    data: {     // fdp.Conf
                        // These all have default values so may be omitted
                        nIntsPerVector: 10,
                        starting_int: -4,
                        ending_int: 14,
                        queue_timeout_ms: 100,
                        wait_between_sends_ms: 1000
                    }
                },
                {
                    inst: "fdc",
                    data: {     // fdc.Conf
                        nIntsPerVector: 10,
                        starting_int: -4,
                        ending_int: 14,
                        queue_timeout_ms: 100,
                    }
                }
            ],
        }
    },
    {
        id: "start",
        data: {
            modules: [          // sequence of...
                {               // ...app::Mod
                    inst: ".*", // match all, empty string also works
                    data: {     // app::Start
                        run: 42,
                    },
                }
            ]
        },
    },
    {
        id: "stop",
        data: {
            modules: [
                {
                    inst: "fdp",
                    data: {},
                },
                {
                    inst: "fdc",
                    data: {},
                },
            ]
        },
    }    
]
