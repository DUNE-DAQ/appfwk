/**
 * @file Application_test.cxx Application class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "rcif/cmd/Nljs.hpp"

#include "Application.hpp"
#include "appfwk/app/Nljs.hpp"
#include "appfwk/cmd/Nljs.hpp"
#include "iomanager/IOManager.hpp"

#define BOOST_TEST_MODULE Application_test // NOLINT

#include "boost/test/unit_test.hpp"

#include <string>
#include <type_traits>

BOOST_AUTO_TEST_SUITE(Application_test)

using namespace dunedaq::appfwk;

const std::string TEST_JSON_FILE = std::string(getenv("DBT_AREA_ROOT")) + "/sourcecode/appfwk/test/scripts/test.json";
const std::string TEST_JSON_DIR = std::string(getenv("DBT_AREA_ROOT")) + "/sourcecode/appfwk/test/scripts/confdata";
const std::string TEST_OKS_DB = "test/config/appSystem.data.xml";

struct EnvFixture
{
  EnvFixture() { setenv("DUNEDAQ_PARTITION", "Application_test", 0); }
};
BOOST_TEST_GLOBAL_FIXTURE(EnvFixture);

BOOST_AUTO_TEST_CASE(Constructor)
{
  Application app(
    "TestApp", "partition_name", "stdin://" + TEST_JSON_FILE, "oksconflibs:" + TEST_OKS_DB);
}

BOOST_AUTO_TEST_CASE(Init)
{
  dunedaq::get_iomanager()->reset();
  Application app(
    "TestApp", "partition_name", "stdin://" + TEST_JSON_FILE, "oksconflibs:" + TEST_OKS_DB);
  app.init();

  dunedaq::iomanager::IOManager::get()->reset();
}

BOOST_AUTO_TEST_CASE(Run)
{
  std::atomic<bool> end_marker = false;

  dunedaq::get_iomanager()->reset();
  Application app(
    "TestApp", "partition_name", "stdin://" + TEST_JSON_FILE, "oksconflibs:" + TEST_OKS_DB);

  BOOST_REQUIRE_EXCEPTION(
    app.run(end_marker), ApplicationNotInitialized, [&](ApplicationNotInitialized) { return true; });

  app.init();

  app.run(end_marker);
  dunedaq::iomanager::IOManager::get()->reset();
}

BOOST_AUTO_TEST_CASE(Start)
{
  dunedaq::get_iomanager()->reset();
  Application app(
    "TestApp", "partition_name", "stdin://" + TEST_JSON_FILE, "oksconflibs:" + TEST_OKS_DB);
  app.init();

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
  dunedaq::rcif::cmd::RCCommand cmd;
  nlohmann::json cmd_data;
  cmd.id = "start";
  cmd.data = start_data;
  cmd.exit_state = "RUNNING";
  to_json(cmd_data, cmd);

  bool cmd_valid = app.is_cmd_valid(cmd_data);
  BOOST_REQUIRE_EQUAL(cmd_valid, true);
  app.execute(cmd_data);
  dunedaq::iomanager::IOManager::get()->reset();
}
BOOST_AUTO_TEST_CASE(Stop)
{
  dunedaq::get_iomanager()->reset();
  Application app(
    "TestApp", "partition_name", "stdin://" + TEST_JSON_FILE, "oksconflibs:" + TEST_OKS_DB);
  app.init();

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

  dunedaq::rcif::cmd::RCCommand cmd;
  nlohmann::json cmd_data;

  cmd.id = "start";
  cmd.data = start_data;
  cmd.exit_state = "RUNNING";
  to_json(cmd_data, cmd);

  bool cmd_valid = app.is_cmd_valid(cmd_data);
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
  dunedaq::iomanager::IOManager::get()->reset();
}

BOOST_AUTO_TEST_CASE(NotInitialized)
{
  dunedaq::get_iomanager()->reset();
  Application app(
    "TestApp", "partition_name", "stdin://" + TEST_JSON_FILE, "oksconflibs:" + TEST_OKS_DB);
  dunedaq::rcif::cmd::RCCommand cmd;
  nlohmann::json cmd_data;

  dunedaq::appfwk::cmd::CmdObj start;
  nlohmann::json start_data;
  to_json(start_data, start);
  cmd.id = "start";
  cmd.data = start_data;
  cmd.exit_state = "RUNNING";
  to_json(cmd_data, cmd);

  bool cmd_valid = app.is_cmd_valid(cmd_data);
  BOOST_REQUIRE_EQUAL(cmd_valid, true);

  BOOST_REQUIRE_EXCEPTION(
    app.execute(cmd_data), DAQModuleManagerNotInitialized, [&](DAQModuleManagerNotInitialized) { return true; });

  cmd_valid = app.is_cmd_valid(cmd_data);
  BOOST_REQUIRE_EQUAL(cmd_valid, false);
  dunedaq::iomanager::IOManager::get()->reset();
}

BOOST_AUTO_TEST_CASE(InvalidCommandTest)
{

  dunedaq::get_iomanager()->reset();
  Application app(
    "TestApp", "partition_name", "stdin://" + TEST_JSON_FILE, "oksconflibs:" + TEST_OKS_DB);
  app.init();

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
  dunedaq::iomanager::IOManager::get()->reset();
}

BOOST_AUTO_TEST_CASE(CommandThrowsException)
{
  dunedaq::get_iomanager()->reset();
  Application app(
    "TestApp", "partition_name", "stdin://" + TEST_JSON_FILE, "oksconflibs:" + TEST_OKS_DB);
  app.init();

  dunedaq::rcif::cmd::RCCommand cmd;
  nlohmann::json cmd_data;

  dunedaq::appfwk::cmd::AddressedCmd addr_cmd;
  addr_cmd.match = "dummy_module_0";

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
  dunedaq::iomanager::IOManager::get()->reset();
}


BOOST_AUTO_TEST_CASE(State)
{
  dunedaq::get_iomanager()->reset();
  Application app(
    "TestApp", "partition_name", "stdin://" + TEST_JSON_FILE, "oksconflibs:" + TEST_OKS_DB);

  std::string state_in = "state";
  app.set_state(state_in);
  std::string state = app.get_state();
  BOOST_REQUIRE_EQUAL(state_in, state);
  dunedaq::iomanager::IOManager::get()->reset();
}

BOOST_AUTO_TEST_SUITE_END()
