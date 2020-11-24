// hand written helpers to make object compliant with appfwk-FakeDataProducerDAQModule-schema
{
    // The internally known name of the only queue used
    queue: "output",

    // Make a conf object for FDP
    conf(nper, beg, end, toms=100, waitms=1000) :: {
        nIntsPerVector: nper, starting_int: beg, ending_int: end,
        queue_timeout_ms: toms, wait_between_sends_ms: waitms
    },
}

