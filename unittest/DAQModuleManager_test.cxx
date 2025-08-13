/**
 * @file DAQModuleManager_test.cxx DAQModuleManager class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "DAQModuleManager.hpp"

#include "appfwk/cmd/Nljs.hpp"

#include "cmdlib/cmd/Nljs.hpp"
#include "iomanager/IOManager.hpp"
#include "opmonlib/TestOpMonManager.hpp"

#define BOOST_TEST_MODULE DAQModuleManager_test // NOLINT

#include "boost/test/unit_test.hpp"

#include <memory>
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
  std::string sessionName = "test-session";
  return std::make_shared<dunedaq::appfwk::ConfigurationManager>(oksConfig, appName, sessionName);
}

BOOST_AUTO_TEST_CASE(Construct)
{
  auto mgr = DAQModuleManager("utest_session");
}

BOOST_AUTO_TEST_CASE(Initialized)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager("utest_session");
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  dunedaq::opmonlib::TestOpMonManager opmgr;
  auto cfgMgr = make_config_mgr();
  mgr.initialize(cfgMgr, opmgr);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);
}

BOOST_AUTO_TEST_CASE(NotInitialized)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager("utest_session");
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  // dunedaq::cmdlib::cmd::Command cmd;
  // cmd.id = "start";
  nlohmann::json cmd_data;
  // to_json(cmd_data, cmd);

  BOOST_REQUIRE_EXCEPTION(mgr.execute("start", cmd_data),
                          DAQModuleManagerNotInitialized,
                          [&](DAQModuleManagerNotInitialized) { return true; });
}

BOOST_AUTO_TEST_CASE(InitializeModules)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager("utest_session");
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  dunedaq::opmonlib::TestOpMonManager opmgr;
  auto cfgMgr = make_config_mgr();
  mgr.initialize(cfgMgr, opmgr);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);
}

BOOST_AUTO_TEST_CASE(NoActionPlan)
{
  if (dunedaq::appfwk::ACTION_PLANS_REQUIRED) {

    dunedaq::get_iomanager()->reset();
    auto mgr = DAQModuleManager("utest_session");
    BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

    dunedaq::opmonlib::TestOpMonManager opmgr;
    auto cfgMgr = make_config_mgr();
    mgr.initialize(cfgMgr, opmgr);

    BOOST_REQUIRE_EQUAL(mgr.initialized(), true);
    nlohmann::json cmd_data;
    BOOST_REQUIRE_EXCEPTION(
      mgr.execute("unknown_cmd", cmd_data), ActionPlanNotFound, [&](ActionPlanNotFound) { return true; });
  }
}

BOOST_AUTO_TEST_CASE(InvalidActionPlan_MissingModuleApp)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager("utest_session");
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  std::string oksConfig = "oksconflibs:test/config/appSession.data.xml";
  std::string appName = "MissingModuleApp";
  std::string sessionName = "test-session";
  dunedaq::opmonlib::TestOpMonManager opmgr;
  auto cfgMgr = std::make_shared<dunedaq::appfwk::ConfigurationManager>(oksConfig, appName, sessionName);

  // This succeeds, but prints a message
  mgr.initialize(cfgMgr, opmgr);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);
  nlohmann::json cmd_data;
  mgr.execute("stuff", cmd_data);

  auto metrics = opmgr.collect();
  auto facility = opmgr.get_backend_facility();
  auto entries = facility->get_entries(std::regex("dunedaq.appfwk.opmon.DummyModuleInfo"));
  BOOST_REQUIRE_EQUAL(entries.size(), 0);
}

BOOST_AUTO_TEST_CASE(InvalidActionPlan_MissingMethodApp)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager("utest_session");
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  std::string oksConfig = "oksconflibs:test/config/appSession.data.xml";
  std::string appName = "MissingMethodApp";
  std::string sessionName = "test-session";
  dunedaq::opmonlib::TestOpMonManager opmgr;
  auto cfgMgr = std::make_shared<dunedaq::appfwk::ConfigurationManager>(oksConfig, appName, sessionName);

  BOOST_REQUIRE_EXCEPTION(
    mgr.initialize(cfgMgr, opmgr), ActionPlanValidationFailed, [&](ActionPlanValidationFailed) { return true; });
}

BOOST_AUTO_TEST_CASE(InvalidActionPlan_ConflictingActionPlansApp)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager("utest_session");
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  std::string oksConfig = "oksconflibs:test/config/appSession.data.xml";
  std::string appName = "ConflictingActionPlansApp";
  std::string sessionName = "test-session";
  dunedaq::opmonlib::TestOpMonManager opmgr;
  auto cfgMgr = std::make_shared<dunedaq::appfwk::ConfigurationManager>(oksConfig, appName, sessionName);

  BOOST_REQUIRE_EXCEPTION(
    mgr.initialize(cfgMgr, opmgr), ActionPlanValidationFailed, [&](ActionPlanValidationFailed) { return true; });
}

BOOST_AUTO_TEST_CASE(InvalidActionPlan_MissingModuleApp_GroupById)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager("utest_session");
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  std::string oksConfig = "oksconflibs:test/config/appSession.data.xml";
  std::string appName = "MissingModuleApp_GroupById";
  std::string sessionName = "test-session";
  dunedaq::opmonlib::TestOpMonManager opmgr;
  auto cfgMgr = std::make_shared<dunedaq::appfwk::ConfigurationManager>(oksConfig, appName, sessionName);

  BOOST_REQUIRE_EXCEPTION(
    mgr.initialize(cfgMgr, opmgr), ActionPlanValidationFailed, [&](ActionPlanValidationFailed) { return true; });
}

BOOST_AUTO_TEST_CASE(InvalidActionPlan_MissingModuleApp_Optional)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager("utest_session");
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  std::string oksConfig = "oksconflibs:test/config/appSession.data.xml";
  std::string appName = "MissingModuleApp_Optional";
  std::string sessionName = "test-session";
  dunedaq::opmonlib::TestOpMonManager opmgr;
  auto cfgMgr = std::make_shared<dunedaq::appfwk::ConfigurationManager>(oksConfig, appName, sessionName);

  // This succeeds, and doesn't print a message
  mgr.initialize(cfgMgr, opmgr);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);
  nlohmann::json cmd_data;
  mgr.execute("stuff", cmd_data);

  auto metrics = opmgr.collect();
  auto facility = opmgr.get_backend_facility();
  auto entries = facility->get_entries(std::regex("dunedaq.appfwk.opmon.DummyModuleInfo"));
  BOOST_REQUIRE_EQUAL(entries.size(), 0);
}

BOOST_AUTO_TEST_CASE(InvalidActionPlan_MissingModuleApp_GroupById_Optional)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager("utest_session");
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  std::string oksConfig = "oksconflibs:test/config/appSession.data.xml";
  std::string appName = "MissingModuleApp_GroupById_Optional";
  std::string sessionName = "test-session";
  dunedaq::opmonlib::TestOpMonManager opmgr;
  auto cfgMgr = std::make_shared<dunedaq::appfwk::ConfigurationManager>(oksConfig, appName, sessionName);

  // This succeeds, and doesn't print a message
  mgr.initialize(cfgMgr, opmgr);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);
  nlohmann::json cmd_data;
  mgr.execute("stuff", cmd_data);
}

BOOST_AUTO_TEST_CASE(InvalidActionPlan_ExtraModuleApp)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager("utest_session");
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  std::string oksConfig = "oksconflibs:test/config/appSession.data.xml";
  std::string appName = "ExtraModuleApp";
  std::string sessionName = "test-session";
  dunedaq::opmonlib::TestOpMonManager opmgr;
  auto cfgMgr = std::make_shared<dunedaq::appfwk::ConfigurationManager>(oksConfig, appName, sessionName);

  // This succeeds, but prints a message
  mgr.initialize(cfgMgr, opmgr);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);
  nlohmann::json cmd_data;
  mgr.execute("stuff", cmd_data);

  auto metrics = opmgr.collect();
  auto facility = opmgr.get_backend_facility();
  auto entries = facility->get_entries(std::regex("dunedaq.appfwk.opmon.DummyModuleInfo"));
  BOOST_REQUIRE_EQUAL(entries.size(), 3);
  uint64_t total_stuff_calls = 0; // NOLINT(build/unsigned)
  for (auto& entry : entries) {
    total_stuff_calls += entry.data().at("stuff_calls").uint8_value(); // NOLINT(build/unsigned)
  }
  BOOST_REQUIRE_EQUAL(total_stuff_calls, 2); // Only 2 of the three modules in the app actually got the stuff command
}

BOOST_AUTO_TEST_CASE(InvalidActionPlan_ExtraModuleApp_GroupById)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager("utest_session");
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  std::string oksConfig = "oksconflibs:test/config/appSession.data.xml";
  std::string appName = "ExtraModuleApp_GroupById";
  std::string sessionName = "test-session";
  dunedaq::opmonlib::TestOpMonManager opmgr;
  auto cfgMgr = std::make_shared<dunedaq::appfwk::ConfigurationManager>(oksConfig, appName, sessionName);

  // This succeeds, but prints a message
  mgr.initialize(cfgMgr, opmgr);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);
  nlohmann::json cmd_data;
  mgr.execute("stuff", cmd_data);

  auto metrics = opmgr.collect();
  auto facility = opmgr.get_backend_facility();
  auto entries = facility->get_entries(std::regex("dunedaq.appfwk.opmon.DummyModuleInfo"));
  BOOST_REQUIRE_EQUAL(entries.size(), 3);
  uint64_t total_stuff_calls = 0; // NOLINT(build/unsigned)
  for (auto& entry : entries) {
    total_stuff_calls += entry.data().at("stuff_calls").uint8_value(); // NOLINT(build/unsigned)
  }
  BOOST_REQUIRE_EQUAL(total_stuff_calls, 2); // Only 2 of the three modules in the app actually got the stuff command
}

BOOST_AUTO_TEST_CASE(CommandModules)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager("utest_session");
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  dunedaq::opmonlib::TestOpMonManager opmgr;
  auto cfgMgr = make_config_mgr();
  mgr.initialize(cfgMgr, opmgr);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);
  nlohmann::json cmd_data;
  mgr.execute("stuff", cmd_data);

  BOOST_REQUIRE_EXCEPTION(
    mgr.execute("bad_stuff", cmd_data), CommandDispatchingFailed, [&](CommandDispatchingFailed) { return true; });
}

BOOST_AUTO_TEST_CASE(CommandModules_ById)
{

  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager("utest_session");
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  std::string oksConfig = "oksconflibs:test/config/appSession.data.xml";
  std::string appName = "TestApp_ById";
  std::string sessionName = "test-session";

  dunedaq::opmonlib::TestOpMonManager opmgr;
  auto cfgMgr = std::make_shared<dunedaq::appfwk::ConfigurationManager>(oksConfig, appName, sessionName);
  mgr.initialize(cfgMgr, opmgr);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);
  nlohmann::json cmd_data;
  mgr.execute("stuff", cmd_data);

  BOOST_REQUIRE_EXCEPTION(
    mgr.execute("bad_stuff", cmd_data), CommandDispatchingFailed, [&](CommandDispatchingFailed) { return true; });
}

BOOST_AUTO_TEST_CASE(CommandMatchingModules)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager("utest_session");
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
  mgr.execute("stuff", cmd_obj_data);

  addr_cmd.match = ".*module.*";
  cmd_obj.modules.push_back(addr_cmd);
  to_json(cmd_obj_data, cmd_obj);
  BOOST_REQUIRE_EXCEPTION(mgr.execute("bad_stuff", cmd_obj_data),
                          ConflictingCommandMatching,
                          [&](ConflictingCommandMatching) { return true; });
}

BOOST_AUTO_TEST_SUITE_END()
