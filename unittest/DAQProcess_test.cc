/**
 * @file DAQProcess class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#include "app-framework/DAQProcess.hh"

#define BOOST_TEST_MODULE DAQProcess_test

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(DAQProcess_test)

BOOST_AUTO_TEST_CASE(Construct) {
    appframework::CommandLineInterpreter args;
    appframework::DAQProcess dp(args);
}

BOOST_AUTO_TEST_SUITE_END()
