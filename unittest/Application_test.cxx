/**
 * @file Application_test.cxx Application class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "rcif/cmd/Nljs.hpp"

#include "appfwk/Application.hpp"
#include "appfwk/QueueRegistry.hpp"
#include "appfwk/app/Nljs.hpp"
#include "appfwk/cmd/Nljs.hpp"

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

BOOST_AUTO_TEST_CASE(Start)
{
  QueueRegistry::reset();
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

  dunedaq::appfwk::cmd::CmdObj start;
  dunedaq::appfwk::cmd::AddressedCmd addr_cmd;
  dunedaq::rcif::cmd::StartParams start_params;
  start_params.run = 1010;
  nlohmann::json start_param_data;
  to_json(start_param_data, start_params);

  addr_cmd.data = start_param_data;
  addr_cmd.match = "";
  start.modules.push_back(addr_cmd);
  nlohmann::json start_data;
  to_json(start_data, start);

  cmd.id = "start";
  cmd.data = start_data;
  cmd.exit_state = "RUNNING";
  to_json(cmd_data, cmd);

  cmd_valid = app.is_cmd_valid(cmd_data);
  BOOST_REQUIRE_EQUAL(cmd_valid, true);
  app.execute(cmd_data);
}
BOOST_AUTO_TEST_CASE(Stop)
{
  QueueRegistry::reset();
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

  dunedaq::rcif::cmd::StartParams start_params;
  start_params.run = 1010;
  nlohmann::json start_param_data;
  to_json(start_param_data, start_params);

  dunedaq::appfwk::cmd::AddressedCmd addr_cmd;
  addr_cmd.data = start_param_data;
  addr_cmd.match = "";

  dunedaq::appfwk::cmd::CmdObj start;
  start.modules.push_back(addr_cmd);
  nlohmann::json start_data;
  to_json(start_data, start);

  cmd.id = "start";
  cmd.data = start_data;
  cmd.exit_state = "RUNNING";
  to_json(cmd_data, cmd);

  cmd_valid = app.is_cmd_valid(cmd_data);
  BOOST_REQUIRE_EQUAL(cmd_valid, true);
  app.execute(cmd_data);

  dunedaq::appfwk::cmd::CmdObj stop;
  nlohmann::json stop_data;
  to_json(stop_data, stop);
  cmd.id = "stop";
  cmd.data = start_data;
  cmd.exit_state = "CONFIGURED";
  to_json(cmd_data, cmd);

  cmd_valid = app.is_cmd_valid(cmd_data);
  BOOST_REQUIRE_EQUAL(cmd_valid, true);
  app.execute(cmd_data);
}

BOOST_AUTO_TEST_CASE(NotInitialized)
{
  Application app("app_name", "partition_name", "stdin://" + TEST_JSON_FILE, "stdout://flat");

  dunedaq::appfwk::cmd::CmdObj start;
  nlohmann::json start_data;
  to_json(start_data, start);
  dunedaq::rcif::cmd::RCCommand cmd;
  cmd.id = "start";
  cmd.data = start_data;
  cmd.exit_state = "RUNNING";
  nlohmann::json cmd_data;
  to_json(cmd_data, cmd);

  bool cmd_valid = app.is_cmd_valid(cmd_data);
  BOOST_REQUIRE_EQUAL(cmd_valid, true);

  BOOST_REQUIRE_EXCEPTION(
    app.execute(cmd_data), DAQModuleManagerNotInitialized, [&](DAQModuleManagerNotInitialized) { return true; });

  cmd_valid = app.is_cmd_valid(cmd_data);
  BOOST_REQUIRE_EQUAL(cmd_valid, false);
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

  cmd_valid = app.is_cmd_valid(cmd_data);
  BOOST_REQUIRE_EQUAL(cmd_valid, false);
}

BOOST_AUTO_TEST_CASE(CommandThrowsException)
{
  QueueRegistry::reset();
  Application app("app_name", "partition_name", "stdin://" + TEST_JSON_FILE, "stdout://flat");

  dunedaq::appfwk::app::Init init;
  dunedaq::appfwk::app::ModSpec module_init;
  module_init.inst = "DummyModule";
  module_init.plugin = "DummyModule";
  init.modules.push_back(module_init);
  nlohmann::json init_data;
  to_json(init_data, init);

  dunedaq::rcif::cmd::RCCommand cmd;
  cmd.id = "init";
  cmd.data = init_data;
  cmd.exit_state = "INITIAL";
  nlohmann::json cmd_data;
  to_json(cmd_data, cmd);

  app.execute(cmd_data);

  dunedaq::appfwk::cmd::AddressedCmd addr_cmd;
  addr_cmd.match = "DummyModule";

  dunedaq::appfwk::cmd::CmdObj cmd_obj;
  cmd_obj.modules.push_back(addr_cmd);
  nlohmann::json cmd_obj_data;
  to_json(cmd_obj_data, cmd_obj);

  cmd.id = "stuff";
  cmd.data = cmd_obj_data;
  to_json(cmd_data, cmd);
  app.execute(cmd_data);

  cmd.id = "bad_stuff";
  to_json(cmd_data, cmd);
  BOOST_REQUIRE_EXCEPTION(
    app.execute(cmd_data), CommandDispatchingFailed, [&](CommandDispatchingFailed) { return true; });
}

BOOST_AUTO_TEST_CASE(Stats)
{
  QueueRegistry::reset();
  Application app("app_name", "partition_name", "stdin://" + TEST_JSON_FILE, "stdout://flat");

  dunedaq::opmonlib::InfoCollector ic;
  app.gather_stats(ic, 0);
  BOOST_REQUIRE(!ic.is_empty());

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

  dunedaq::appfwk::cmd::CmdObj start;
  dunedaq::appfwk::cmd::AddressedCmd addr_cmd;
  dunedaq::rcif::cmd::StartParams start_params;
  start_params.run = 1010;
  nlohmann::json start_param_data;
  to_json(start_param_data, start_params);

  addr_cmd.data = start_param_data;
  addr_cmd.match = "";
  start.modules.push_back(addr_cmd);
  nlohmann::json start_data;
  to_json(start_data, start);

  cmd.id = "start";
  cmd.data = start_data;
  cmd.exit_state = "RUNNING";
  to_json(cmd_data, cmd);

  cmd_valid = app.is_cmd_valid(cmd_data);
  BOOST_REQUIRE_EQUAL(cmd_valid, true);
  app.execute(cmd_data);

  app.gather_stats(ic, 999);
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
