// hand written helpers to make object compliant with appfwk-fdc-schema
{
    // Make a conf object for FDC
    conf(nper, beg, end, toms=100) :: {
        nIntsPerVector: nper, starting_int: beg, ending_int: end,
        queue_timeout_ms: toms, 
    },
}
