/**
 * @file CommandLineInterpreter_test.cxx CommandLineInterpreter class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/CommandLineInterpreter.hpp"

#define BOOST_TEST_MODULE CommandLineInterpreter_test // NOLINT

#include "boost/test/unit_test.hpp"

#include <string>
#include <vector>

using namespace dunedaq::appfwk;

BOOST_AUTO_TEST_SUITE(CommandLineInterpreter_test)

BOOST_AUTO_TEST_CASE(ParseNoOptions)
{
  char** arg_list = new char* [1] { (char*)("CommandLineInterpreter_test") };
  BOOST_REQUIRE_EXCEPTION(CommandLineInterpreter::parse(1, arg_list),
                          dunedaq::appfwk::CommandLineIssue,
                          [&](dunedaq::appfwk::CommandLineIssue) { return true; });
}

BOOST_AUTO_TEST_CASE(AskForHelp) {

  char** arg_list = new char* [2] {
    (char*)("CommandLineInterpreter_test"), (char*)("-h")  };
  auto parsed = CommandLineInterpreter::parse(2, arg_list);

  BOOST_REQUIRE_EQUAL(parsed.help_requested, true);
  BOOST_REQUIRE_EQUAL(parsed.app_name, "");
  BOOST_REQUIRE_EQUAL(parsed.partition_name, "");
  BOOST_REQUIRE_EQUAL(parsed.command_facility_plugin_name, "");
  BOOST_REQUIRE_EQUAL(parsed.info_service_plugin_name, "");
  BOOST_REQUIRE_EQUAL(parsed.other_options.size(), 0);
}

BOOST_AUTO_TEST_CASE(ParseCommandFacility)
{
  char** arg_list = new char* [3] { (char*)("CommandLineInterpreter_test"), (char*)("-c"), (char*)("stdin://") };
  BOOST_REQUIRE_EXCEPTION(CommandLineInterpreter::parse(3, arg_list),
                          dunedaq::appfwk::CommandLineIssue,
                          [&](dunedaq::appfwk::CommandLineIssue) { return true; });
}

BOOST_AUTO_TEST_CASE(ParseName)
{

  char** arg_list = new char* [3] { (char*)("CommandLineInterpreter_test"), (char*)("-n"), (char*)("cli_test") };
  BOOST_REQUIRE_EXCEPTION(CommandLineInterpreter::parse(3, arg_list),
                          dunedaq::appfwk::CommandLineIssue,
                          [&](dunedaq::appfwk::CommandLineIssue) { return true; });
}

BOOST_AUTO_TEST_CASE(ParseNameAndCommandFacility)
{
  char** arg_list = new char* [5] { (char*)("CommandLineInterpreter_test"), 
      (char*)("-c"), (char*)("stdin://"), 
      (char*)("-n"), (char*)("cli_test") };
  auto parsed = CommandLineInterpreter::parse(5, arg_list);

  BOOST_REQUIRE_EQUAL(parsed.help_requested, false);
  BOOST_REQUIRE_EQUAL(parsed.app_name, "cli_test");
  BOOST_REQUIRE_EQUAL(parsed.partition_name, "global");
  BOOST_REQUIRE_EQUAL(parsed.command_facility_plugin_name, "stdin://");
  BOOST_REQUIRE_EQUAL(parsed.info_service_plugin_name, "stdout://flat");
  BOOST_REQUIRE_EQUAL(parsed.other_options.size(), 0);
}
BOOST_AUTO_TEST_CASE(ParseInfoService)
{
  char** arg_list = new char* [7] {
    (char*)("CommandLineInterpreter_test"),
    (char*)("-c"), (char*)("stdin://"), 
    (char*)("-n"), (char*)("cli_test"),
      (char*)("-i"), (char*)("info_service://")
  };
  auto parsed = CommandLineInterpreter::parse(7, arg_list);

  BOOST_REQUIRE_EQUAL(parsed.help_requested, false);
  BOOST_REQUIRE_EQUAL(parsed.app_name, "cli_test");
  BOOST_REQUIRE_EQUAL(parsed.partition_name, "global");
  BOOST_REQUIRE_EQUAL(parsed.command_facility_plugin_name, "stdin://");
  BOOST_REQUIRE_EQUAL(parsed.info_service_plugin_name, "info_service://");
  BOOST_REQUIRE_EQUAL(parsed.other_options.size(), 0);
}
BOOST_AUTO_TEST_CASE(ParsePartition)
{
  char** arg_list = new char* [7] {
    (char*)("CommandLineInterpreter_test"), 
    (char*)("-c"), (char*)("stdin://"), 
    (char*)("-n"), (char*)("cli_test"),
      (char*)("-p"), (char*)("test_partition")
  };
  auto parsed = CommandLineInterpreter::parse(7, arg_list);

  BOOST_REQUIRE_EQUAL(parsed.help_requested, false);
  BOOST_REQUIRE_EQUAL(parsed.app_name, "cli_test");
  BOOST_REQUIRE_EQUAL(parsed.partition_name, "test_partition");
  BOOST_REQUIRE_EQUAL(parsed.command_facility_plugin_name, "stdin://");
  BOOST_REQUIRE_EQUAL(parsed.info_service_plugin_name, "stdout://flat");
  BOOST_REQUIRE_EQUAL(parsed.other_options.size(), 0);
}
BOOST_AUTO_TEST_CASE(ParseOtherOption)
{
  char** arg_list = new char* [6] {
    (char*)("CommandLineInterpreter_test"), 
    (char*)("-c"), (char*)("stdin://"), 
    (char*)("-n"), (char*)("cli_test"),
      (char*)("--some-other-option")
  };
  auto parsed = CommandLineInterpreter::parse(6, arg_list);

  BOOST_REQUIRE_EQUAL(parsed.help_requested, false);
  BOOST_REQUIRE_EQUAL(parsed.app_name, "cli_test");
  BOOST_REQUIRE_EQUAL(parsed.partition_name, "global");
  BOOST_REQUIRE_EQUAL(parsed.command_facility_plugin_name, "stdin://");
  BOOST_REQUIRE_EQUAL(parsed.info_service_plugin_name, "stdout://flat");
  BOOST_REQUIRE_EQUAL(parsed.other_options.size(), 1);
  BOOST_REQUIRE_EQUAL(parsed.other_options[0], "--some-other-option");
}
BOOST_AUTO_TEST_CASE(ParseMultipleOtherOptions)
{
  char** arg_list = new char* [9] {
    (char*)("CommandLineInterpreter_test"),
    (char*)("-c"), (char*)("stdin://"), 
    (char*)("-n"), (char*)("cli_test"),
      (char*)("--some-other-option"), 
      (char*)("--yet-another-option=4"), 
      (char*)("-u"), (char*)("me")
  };
  auto parsed = CommandLineInterpreter::parse(9, arg_list);

  BOOST_REQUIRE_EQUAL(parsed.help_requested, false);
  BOOST_REQUIRE_EQUAL(parsed.app_name, "cli_test");
  BOOST_REQUIRE_EQUAL(parsed.partition_name, "global");
  BOOST_REQUIRE_EQUAL(parsed.command_facility_plugin_name, "stdin://");
  BOOST_REQUIRE_EQUAL(parsed.info_service_plugin_name, "stdout://flat");
  BOOST_REQUIRE_EQUAL(parsed.other_options.size(), 4);
  BOOST_REQUIRE_EQUAL(parsed.other_options[0], "--some-other-option");
  BOOST_REQUIRE_EQUAL(parsed.other_options[1], "--yet-another-option=4");
  BOOST_REQUIRE_EQUAL(parsed.other_options[2], "-u");
  BOOST_REQUIRE_EQUAL(parsed.other_options[3], "me");
}

BOOST_AUTO_TEST_SUITE_END()
