/**
 * @file DAQModuleManager_test.cxx DAQModuleManager class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "cmdlib/cmd/Nljs.hpp"

#include "apps/DAQModuleManager.hpp"
#include "appfwk/Issues.hpp"
#include "appfwk/QueueRegistry.hpp"
#include "appfwk/app/Nljs.hpp"
#include "appfwk/cmd/Nljs.hpp"

#define BOOST_TEST_MODULE DAQModuleManager_test // NOLINT

#include "boost/test/unit_test.hpp"

#include <string>
#include <type_traits>

BOOST_AUTO_TEST_SUITE(DAQModuleManager_test)

using namespace dunedaq::appfwk;

BOOST_AUTO_TEST_CASE(Construct)
{
  auto mgr = DAQModuleManager();
}

BOOST_AUTO_TEST_CASE(Initialized)
{
  QueueRegistry::reset();
  auto mgr = DAQModuleManager();
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  dunedaq::appfwk::app::Init init;
  nlohmann::json init_data;
  to_json(init_data, init);
  dunedaq::cmdlib::cmd::Command cmd;
  cmd.id = "init";
  cmd.data = init_data;
  nlohmann::json cmd_data;
  to_json(cmd_data, cmd);
  mgr.execute(cmd_data);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);

  BOOST_REQUIRE_EXCEPTION(mgr.execute(cmd_data),
                          DAQModuleManagerAlreadyInitialized,
                          [&](DAQModuleManagerAlreadyInitialized) { return true; });
}

BOOST_AUTO_TEST_CASE(NotInitialized)
{

  QueueRegistry::reset();
  auto mgr = DAQModuleManager();
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  dunedaq::cmdlib::cmd::Command cmd;
  cmd.id = "start";
  nlohmann::json cmd_data;
  to_json(cmd_data, cmd);

  BOOST_REQUIRE_EXCEPTION(
    mgr.execute(cmd_data), DAQModuleManagerNotInitialized, [&](DAQModuleManagerNotInitialized) { return true; });
}

BOOST_AUTO_TEST_CASE(Stats)
{
  auto mgr = DAQModuleManager();
  dunedaq::opmonlib::InfoCollector ic;
  mgr.gather_stats(ic, 0);
}

BOOST_AUTO_TEST_CASE(InitializeModules)
{
  QueueRegistry::reset();
  auto mgr = DAQModuleManager();
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  dunedaq::appfwk::app::Init init;
  dunedaq::appfwk::app::ModSpec module_init;
  module_init.inst = "DummyModule";
  module_init.plugin = "DummyModule";
  init.modules.push_back(module_init);
  nlohmann::json init_data;
  to_json(init_data, init);
  dunedaq::cmdlib::cmd::Command cmd;
  cmd.id = "init";
  cmd.data = init_data;
  nlohmann::json cmd_data;
  to_json(cmd_data, cmd);
  mgr.execute(cmd_data);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);
}

BOOST_AUTO_TEST_CASE(CommandModules)
{
  QueueRegistry::reset();
  auto mgr = DAQModuleManager();
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  dunedaq::appfwk::app::Init init;
  dunedaq::appfwk::app::ModSpec module_init;
  module_init.inst = "DummyModule";
  module_init.plugin = "DummyModule";
  init.modules.push_back(module_init);
  nlohmann::json init_data;
  to_json(init_data, init);
  dunedaq::cmdlib::cmd::Command cmd;
  cmd.id = "init";
  cmd.data = init_data;
  nlohmann::json cmd_data;
  to_json(cmd_data, cmd);
  mgr.execute(cmd_data);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);

  cmd.id = "stuff";
  to_json(cmd_data, cmd);
  mgr.execute(cmd_data);

  cmd.id = "bad_stuff";
  to_json(cmd_data, cmd);
  BOOST_REQUIRE_EXCEPTION(
    mgr.execute(cmd_data), CommandDispatchingFailed, [&](CommandDispatchingFailed) { return true; });

  dunedaq::opmonlib::InfoCollector ic;
  mgr.gather_stats(ic, 0);
}

BOOST_AUTO_TEST_CASE(CommandMatchingModules)
{
  QueueRegistry::reset();
  auto mgr = DAQModuleManager();
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  dunedaq::appfwk::app::Init init;
  dunedaq::appfwk::app::ModSpec module_init;
  module_init.inst = "DummyModule";
  module_init.plugin = "DummyModule";
  init.modules.push_back(module_init);
  nlohmann::json init_data;
  to_json(init_data, init);
  dunedaq::cmdlib::cmd::Command cmd;
  cmd.id = "init";
  cmd.data = init_data;
  nlohmann::json cmd_data;
  to_json(cmd_data, cmd);
  mgr.execute(cmd_data);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);

  dunedaq::appfwk::cmd::CmdObj cmd_obj;
  nlohmann::json cmd_obj_data;
  dunedaq::appfwk::cmd::AddressedCmd addr_cmd;
  addr_cmd.match = "Dummy.*";
  cmd_obj.modules.push_back(addr_cmd);
  addr_cmd.match = "foo";
  cmd_obj.modules.push_back(addr_cmd);
  to_json(cmd_obj_data, cmd_obj);
  cmd.id = "stuff";
  cmd.data = cmd_obj_data;
  to_json(cmd_data, cmd);
  mgr.execute(cmd_data);

  addr_cmd.match = ".*Module";
  cmd_obj.modules.push_back(addr_cmd);
  to_json(cmd_obj_data, cmd_obj);
  cmd.id = "stuff";
  cmd.data = cmd_obj_data;
  to_json(cmd_data, cmd);
  BOOST_REQUIRE_EXCEPTION(
    mgr.execute(cmd_data), ConflictingCommandMatching, [&](ConflictingCommandMatching) { return true; });
}

BOOST_AUTO_TEST_CASE(InitializeQueues)
{
  QueueRegistry::reset();
  auto mgr = DAQModuleManager();
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  dunedaq::appfwk::app::Init init;
  dunedaq::appfwk::app::QueueSpec queue_init;
  queue_init.kind = dunedaq::appfwk::app::QueueKind::StdDeQueue;
  queue_init.inst = "test_deque";
  queue_init.capacity = 10;
  init.queues.push_back(queue_init);
  queue_init.kind = dunedaq::appfwk::app::QueueKind::FollySPSCQueue;
  queue_init.inst = "test_spsc";
  init.queues.push_back(queue_init);
  queue_init.kind = dunedaq::appfwk::app::QueueKind::FollyMPMCQueue;
  queue_init.inst = "test_mpmc";
  init.queues.push_back(queue_init);
  nlohmann::json init_data;
  to_json(init_data, init);
  dunedaq::cmdlib::cmd::Command cmd;
  cmd.id = "init";
  cmd.data = init_data;
  nlohmann::json cmd_data;
  to_json(cmd_data, cmd);
  mgr.execute(cmd_data);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);
}

BOOST_AUTO_TEST_CASE(InitializeUnknownQueueType)
{
  QueueRegistry::reset();
  auto mgr = DAQModuleManager();

  dunedaq::appfwk::app::Init init;
  dunedaq::appfwk::app::QueueSpec queue_init;
  queue_init.kind = dunedaq::appfwk::app::QueueKind::Unknown;
  queue_init.inst = "test_invalid";
  queue_init.capacity = 10;
  init.queues.push_back(queue_init);
  nlohmann::json init_data;
  to_json(init_data, init);

  dunedaq::cmdlib::cmd::Command cmd;
  cmd.id = "init";
  cmd.data = init_data;
  nlohmann::json cmd_data;
  to_json(cmd_data, cmd);

  BOOST_REQUIRE_EXCEPTION(mgr.execute(cmd_data), MissingComponent, [&](MissingComponent) { return true; });
}

BOOST_AUTO_TEST_SUITE_END()
