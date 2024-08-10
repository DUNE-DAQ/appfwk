/**
 * @file DAQModuleManager_test.cxx DAQModuleManager class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "cmdlib/cmd/Nljs.hpp"

#include "DAQModuleManager.hpp"
#include "appfwk/Issues.hpp"
#include "appfwk/app/Nljs.hpp"
#include "appfwk/cmd/Nljs.hpp"
#include "iomanager/connection/Nljs.hpp"
#include "opmonlib/TestOpMonManager.hpp"

#include "iomanager/IOManager.hpp"

#define BOOST_TEST_MODULE DAQModuleManager_test // NOLINT

#include "boost/test/unit_test.hpp"

#include <string>
#include <type_traits>

BOOST_AUTO_TEST_SUITE(DAQModuleManager_test)

using namespace dunedaq::appfwk;

struct EnvFixture
{
  EnvFixture() { setenv("DUNEDAQ_PARTITION", "DAQModulesManager_test", 0); }
};
BOOST_TEST_GLOBAL_FIXTURE(EnvFixture);

std::shared_ptr<dunedaq::appfwk::ConfigurationManager>
make_config_mgr()
{
  std::string oksConfig = "oksconflibs:test/config/appSession.data.xml";
  std::string appName = "TestApp";
  std::string sessionName = "partition_name";
  return std::make_shared<dunedaq::appfwk::ConfigurationManager>(oksConfig, appName, sessionName);
}

BOOST_AUTO_TEST_CASE(Construct)
{
  auto mgr = DAQModuleManager();
}

BOOST_AUTO_TEST_CASE(Initialized)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager();
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  dunedaq::opmonlib::TestOpMonManager opmgr;
  auto cfgMgr = make_config_mgr();
  mgr.initialize(cfgMgr, opmgr);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);
}

BOOST_AUTO_TEST_CASE(NotInitialized)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager();
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  // dunedaq::cmdlib::cmd::Command cmd;
  // cmd.id = "start";
  nlohmann::json cmd_data;
  // to_json(cmd_data, cmd);

  BOOST_REQUIRE_EXCEPTION(mgr.execute("CONFIGURED", "start", cmd_data),
                          DAQModuleManagerNotInitialized,
                          [&](DAQModuleManagerNotInitialized) { return true; });
}


BOOST_AUTO_TEST_CASE(InitializeModules)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager();
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  dunedaq::opmonlib::TestOpMonManager opmgr;
  auto cfgMgr = make_config_mgr();
  mgr.initialize(cfgMgr, opmgr);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);
}

BOOST_AUTO_TEST_CASE(CommandModules)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager();
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  dunedaq::opmonlib::TestOpMonManager opmgr;
  auto cfgMgr = make_config_mgr();
  mgr.initialize(cfgMgr, opmgr);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);
  nlohmann::json cmd_data;
  mgr.execute("RUNNING", "stuff", cmd_data);

  BOOST_REQUIRE_EXCEPTION(mgr.execute("RUNNING", "bad_stuff", cmd_data),
                          CommandDispatchingFailed,
                          [&](CommandDispatchingFailed) { return true; });

}

BOOST_AUTO_TEST_CASE(CommandMatchingModules)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager();
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  dunedaq::opmonlib::TestOpMonManager opmgr;
  auto cfgMgr = make_config_mgr();
  mgr.initialize(cfgMgr, opmgr);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);

  dunedaq::appfwk::cmd::CmdObj cmd_obj;
  nlohmann::json cmd_obj_data;
  dunedaq::appfwk::cmd::AddressedCmd addr_cmd;
  addr_cmd.match = "dummy.*";
  cmd_obj.modules.push_back(addr_cmd);
  addr_cmd.match = "foo";
  cmd_obj.modules.push_back(addr_cmd);
  to_json(cmd_obj_data, cmd_obj);
  mgr.execute("RUNNING", "stuff", cmd_obj_data);

  addr_cmd.match = ".*module.*";
  cmd_obj.modules.push_back(addr_cmd);
  to_json(cmd_obj_data, cmd_obj);
  BOOST_REQUIRE_EXCEPTION(mgr.execute("RUNNING", "bad_stuff", cmd_obj_data),
                          ConflictingCommandMatching,
                          [&](ConflictingCommandMatching) { return true; });
}

BOOST_AUTO_TEST_SUITE_END()
