[
    {
        id: "init",
        data: {                 // Addressed, passed to initialize()
            addrdats: [
                {               // Addrdat in initialize()
                    ki: {
                        kind: "queue",
                        inst: "IGNORED",
                    },
                    data: {         // Addressed, passed to init_queues()
                        addrdats: [
                            {
                                ki: {
                                    kind: "IGNORED", // actual parameter
                                    inst: "hose",  // actual parameter
                                },
                                data: { // QueueInit
                                    kind: "StdDeQueue", // should be in ki.kind
                                    capacity: 10,       // acutal param
                                },
                            },
                        ],
                    },
                },
                {               // AddrDat in initialize()
                    ki: {
                        kind: "module",
                        inst: "fdp",
                    },
                    data: {     // ModInit
                        plugin: "FakeDataProducerDAQModule",
                        data: { // fdp.Init
                            output: "hose",
                        }
                    },
                },
                {
                    ki: {
                        kind: "module",
                        inst: "fdc",
                    },
                    data: { // ModInit
                        plugin: "FakeDataConsumerDAQModule",
                        data: { // fdc.Init
                            input: "hose",
                        },
                    },
                }
            ],
        },
    },
    {                           // 
        id: "conf",
        data: {                 // Addressed
            addrdats: [
                {               // AddrDat
                    ki: {
                        kind: "module",
                        inst: "fdp",
                    },
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
                    ki: {
                        kind: "module",
                        inst: "fdc",
                    },
                    data: {
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
            addrdats: [
                {
                    ki: {
                        kind: "module",
                        inst: "fdp",
                    },
                    data: {}
                },
                {
                    ki: {
                        kind: "module",
                        inst: "fdc",
                    },
                    data: { }
                }
            ]
        },
    },
    {
        id: "stop",
        data: {
            addrdats: [
                {
                    ki: {
                        kind: "module",
                        inst: "fdp",
                    },
                    data: {}
                },
                {
                    ki: {
                        kind: "module",
                        inst: "fdc",
                    },
                    data: { }
                }
            ]
        },
    }
    
]
