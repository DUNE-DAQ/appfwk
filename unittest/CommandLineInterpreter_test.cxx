/**
 * @file CommandLineInterpreter_test.cxx CommandLineInterpreter class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "CommandLineInterpreter.hpp"

#define BOOST_TEST_MODULE CommandLineInterpreter_test // NOLINT

#include "boost/test/unit_test.hpp"

#include <string>
#include <vector>

using namespace dunedaq::appfwk;

// n.b. Because CommandLineInterpreter requires a non-const char**,
// some coding guidelines need to be bent for the sake of simplicity

BOOST_AUTO_TEST_SUITE(CommandLineInterpreter_test)

BOOST_AUTO_TEST_CASE(ParseNoOptions)
{
  char** arg_list = new char* [1] { (char*)("CommandLineInterpreter_test") }; // NOLINT
  BOOST_REQUIRE_EXCEPTION(CommandLineInterpreter::parse(1, arg_list),
                          bpo::error,
			  [&](bpo::error){ return true;} );


  delete[] arg_list; // NOLINT
}

BOOST_AUTO_TEST_CASE(AskForHelp)
{

  char** arg_list = new char* [2] { (char*)("CommandLineInterpreter_test"), (char*)("-h") }; // NOLINT
  auto parsed = CommandLineInterpreter::parse(2, arg_list);

  BOOST_REQUIRE_EQUAL(parsed.help_requested, true);
  BOOST_REQUIRE_EQUAL(parsed.app_name, "");
  BOOST_REQUIRE_EQUAL(parsed.session_name, "");
  BOOST_REQUIRE_EQUAL(parsed.command_facility_plugin_name, "");
  BOOST_REQUIRE_EQUAL(parsed.conf_service_plugin_name, "");
  BOOST_REQUIRE_EQUAL(parsed.other_options.size(), 0);

  delete[] arg_list; // NOLINT
}

BOOST_AUTO_TEST_CASE(ParseCommandFacility)
{
  char** arg_list =
    new char* [3] { (char*)("CommandLineInterpreter_test"), (char*)("-c"), (char*)("stdin://") }; // NOLINT
  BOOST_REQUIRE_EXCEPTION(CommandLineInterpreter::parse(3, arg_list),
                          bpo::error,
			  [&](bpo::error){ return true;} );

  delete[] arg_list; // NOLINT
}

BOOST_AUTO_TEST_CASE(ParseName)
{

  char** arg_list = new char* [5] {
    (char*)("CommandLineInterpreter_test"), (char*)("-n"), (char*)("cli_test"), (char*)("-d"), (char*)("file://"),
  }; // NOLINT
  BOOST_REQUIRE_EXCEPTION(CommandLineInterpreter::parse(5, arg_list),
                          bpo::error,
			  [&](bpo::error){ return true;} );

  delete[] arg_list; // NOLINT
}

BOOST_AUTO_TEST_CASE(ParseNameAndCommandFacility)
{
  char** arg_list = new char* [9] {
    (char*)("CommandLineInterpreter_test"),   // NOLINT
      (char*)("-c"), (char*)("stdin://"),     // NOLINT
      (char*)("-d"), (char*)("file://"),      // NOLINT
      (char*)("-s"), (char*)("test_session"), // NOLINT
      (char*)("-n"), (char*)("cli_test")      // NOLINT
  };
  auto parsed = CommandLineInterpreter::parse(9, arg_list);

  BOOST_REQUIRE_EQUAL(parsed.help_requested, false);
  BOOST_REQUIRE_EQUAL(parsed.app_name, "cli_test");
  BOOST_REQUIRE_EQUAL(parsed.session_name, "test_session");
  BOOST_REQUIRE_EQUAL(parsed.command_facility_plugin_name, "stdin://");
  BOOST_REQUIRE_EQUAL(parsed.conf_service_plugin_name, "file://");
  BOOST_REQUIRE_EQUAL(parsed.other_options.size(), 0);

  delete[] arg_list; // NOLINT
}

BOOST_AUTO_TEST_CASE(ParseSession)
{
  char** arg_list = new char* [9] {
    (char*)("CommandLineInterpreter_test"),    // NOLINT
      (char*)("-c"), (char*)("stdin://"),      // NOLINT
      (char*)("-d"), (char*)("file://"),       // NOLINT
      (char*)("-n"), (char*)("cli_test"),      // NOLINT
      (char*)("-s"), (char*)("test_session")   // NOLINT
  };
  auto parsed = CommandLineInterpreter::parse(9, arg_list);

  BOOST_REQUIRE_EQUAL(parsed.help_requested, false);
  BOOST_REQUIRE_EQUAL(parsed.app_name, "cli_test");
  BOOST_REQUIRE_EQUAL(parsed.session_name, "test_session");
  BOOST_REQUIRE_EQUAL(parsed.command_facility_plugin_name, "stdin://");
  BOOST_REQUIRE_EQUAL(parsed.conf_service_plugin_name, "file://");
  BOOST_REQUIRE_EQUAL(parsed.other_options.size(), 0);

  delete[] arg_list; // NOLINT
}
BOOST_AUTO_TEST_CASE(ParseOtherOption)
{
  char** arg_list = new char* [10] {
    (char*)("CommandLineInterpreter_test"),   // NOLINT
      (char*)("-c"), (char*)("stdin://"),     // NOLINT
      (char*)("-d"), (char*)("file://"),      // NOLINT
      (char*)("-n"), (char*)("cli_test"),     // NOLINT
      (char*)("-s"), (char*)("test_session"), // NOLINT
      (char*)("--some-other-option")          // NOLINT
  };
  auto parsed = CommandLineInterpreter::parse(10, arg_list);

  BOOST_REQUIRE_EQUAL(parsed.help_requested, false);
  BOOST_REQUIRE_EQUAL(parsed.app_name, "cli_test");
  BOOST_REQUIRE_EQUAL(parsed.session_name, "test_session");
  BOOST_REQUIRE_EQUAL(parsed.command_facility_plugin_name, "stdin://");
  BOOST_REQUIRE_EQUAL(parsed.conf_service_plugin_name, "file://");
  BOOST_REQUIRE_EQUAL(parsed.other_options.size(), 1);
  BOOST_REQUIRE_EQUAL(parsed.other_options[0], "--some-other-option");

  delete[] arg_list; // NOLINT
}
BOOST_AUTO_TEST_CASE(ParseMultipleOtherOptions)
{
  char** arg_list = new char* [13] {
    (char*)("CommandLineInterpreter_test"),   // NOLINT
      (char*)("-c"), (char*)("stdin://"),     // NOLINT
      (char*)("-d"), (char*)("file://"),      // NOLINT
      (char*)("-n"), (char*)("cli_test"),     // NOLINT
      (char*)("-s"), (char*)("test_session"), // NOLINT
      (char*)("--some-other-option"),         // NOLINT
      (char*)("--yet-another-option=4"),      // NOLINT
      (char*)("-u"), (char*)("me")            // NOLINT
  };
  auto parsed = CommandLineInterpreter::parse(13, arg_list);

  BOOST_REQUIRE_EQUAL(parsed.help_requested, false);
  BOOST_REQUIRE_EQUAL(parsed.app_name, "cli_test");
  BOOST_REQUIRE_EQUAL(parsed.session_name, "test_session");
  BOOST_REQUIRE_EQUAL(parsed.command_facility_plugin_name, "stdin://");
  BOOST_REQUIRE_EQUAL(parsed.conf_service_plugin_name, "file://");
  BOOST_REQUIRE_EQUAL(parsed.other_options.size(), 4);
  BOOST_REQUIRE_EQUAL(parsed.other_options[0], "--some-other-option");
  BOOST_REQUIRE_EQUAL(parsed.other_options[1], "--yet-another-option=4");
  BOOST_REQUIRE_EQUAL(parsed.other_options[2], "-u");
  BOOST_REQUIRE_EQUAL(parsed.other_options[3], "me");
  delete[] arg_list; // NOLINT
}

BOOST_AUTO_TEST_SUITE_END()
