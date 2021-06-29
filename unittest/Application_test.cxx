/**
 * @file Application_test.cxx Application class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "rcif/cmd/Nljs.hpp"

#include "appfwk/Application.hpp"
#include "appfwk/app/Nljs.hpp"

#define BOOST_TEST_MODULE Application_test // NOLINT

#include "boost/test/unit_test.hpp"

#include <string>
#include <type_traits>

BOOST_AUTO_TEST_SUITE(Application_test)

using namespace dunedaq::appfwk;

const std::string TEST_JSON_FILE = std::string(getenv("APPFWK_SHARE")) + "/test/scripts/test.json";

BOOST_AUTO_TEST_CASE(Constructor)
{

  Application app("app_name", "partition_name", "stdin://" + TEST_JSON_FILE, "stdout://flat");
}

BOOST_AUTO_TEST_CASE(Init)
{
  Application app("app_name", "partition_name", "stdin://" + TEST_JSON_FILE, "stdout://flat");
  app.init();
}

BOOST_AUTO_TEST_CASE(Run)
{
  std::atomic<bool> end_marker = false;

  Application app("app_name", "partition_name", "stdin://" + TEST_JSON_FILE, "stdout://flat");

  BOOST_REQUIRE_EXCEPTION(
    app.run(end_marker), ApplicationNotInitialized, [&](ApplicationNotInitialized) { return true; });

  app.init();

  app.run(end_marker);
}

BOOST_AUTO_TEST_CASE(Execute)
{
  Application app("app_name", "partition_name", "stdin://" + TEST_JSON_FILE, "stdout://flat");

  dunedaq::appfwk::app::Init init;
  nlohmann::json init_data;
  to_json(init_data, init);
  dunedaq::rcif::cmd::RCCommand cmd;
  cmd.id = "init";
  cmd.data = init_data;
  cmd.exit_state = "INITIAL";
  nlohmann::json cmd_data;
  to_json(cmd_data, cmd);

  bool cmd_valid = app.is_cmd_valid(cmd_data);
  BOOST_REQUIRE_EQUAL(cmd_valid, true);

  app.execute(cmd_data);
}

BOOST_AUTO_TEST_CASE(InvalidCommandTest)
{

  Application app("app_name", "partition_name", "stdin://" + TEST_JSON_FILE, "stdout://flat");

  dunedaq::rcif::cmd::RCCommand cmd;
  cmd.id = "badCommand";
  cmd.entry_state = "NonexistentState";
  nlohmann::json cmd_data;
  to_json(cmd_data, cmd);

  bool cmd_valid = app.is_cmd_valid(cmd_data);
  BOOST_REQUIRE_EQUAL(cmd_valid, false);

  BOOST_REQUIRE_EXCEPTION(app.execute(cmd_data), InvalidCommand, [&](InvalidCommand) { return true; });
}

BOOST_AUTO_TEST_CASE(Stats)
{
  Application app("app_name", "partition_name", "stdin://" + TEST_JSON_FILE, "stdout://flat");

  dunedaq::opmonlib::InfoCollector ic;
  app.gather_stats(ic, 0);
  BOOST_REQUIRE(!ic.is_empty());
}

BOOST_AUTO_TEST_CASE(State)
{
  Application app("app_name", "partition_name", "stdin://" + TEST_JSON_FILE, "stdout://flat");

  std::string state_in = "state";
  app.set_state(state_in);
  std::string state = app.get_state();
  BOOST_REQUIRE_EQUAL(state_in, state);
}

BOOST_AUTO_TEST_SUITE_END()
