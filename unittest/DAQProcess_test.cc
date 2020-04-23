/**
 * @file DAQProcess class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#include "app-framework/DAQProcess.hh"
#include "app-framework/QueryResponseCommandFacility.hh"

#define BOOST_TEST_MODULE DAQProcess_test  

#include <boost/test/auto_unit_test.hpp>


namespace appframework {
std::unique_ptr<CommandFacility> CommandFacility::handle_ = std::unique_ptr<CommandFacility>(new QueryResponseCommandFacility());
} // namespace appframework


BOOST_AUTO_TEST_SUITE(DAQProcess_test)

BOOST_AUTO_TEST_CASE(Construct) {
    std::list<std::string> args;
    appframework::DAQProcess dp(args);
}

BOOST_AUTO_TEST_SUITE_END()
