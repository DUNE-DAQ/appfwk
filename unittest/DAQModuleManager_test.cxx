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

BOOST_AUTO_TEST_CASE(Construct)
{
  auto mgr = DAQModuleManager();
}

BOOST_AUTO_TEST_CASE(Initialized)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager();
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  mgr.initialize(nullptr);

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

BOOST_AUTO_TEST_CASE(Stats)
{
  auto mgr = DAQModuleManager();
  dunedaq::opmonlib::InfoCollector ic;
  mgr.gather_stats(ic, 0);
}

BOOST_AUTO_TEST_CASE(InitializeModules)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager();
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  dunedaq::appfwk::app::Init init;
  dunedaq::appfwk::app::ModSpec module_init;
  module_init.inst = "DummyModule";
  module_init.plugin = "DummyModule";
  init.modules.push_back(module_init);
  nlohmann::json init_data;
  to_json(init_data, init);
  // dunedaq::cmdlib::cmd::Command cmd;
  // cmd.id = "init";
  // cmd.data = init_data;
  // nlohmann::json cmd_data;
  // to_json(cmd_data, cmd);
  mgr.initialize(init_data);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);
}

BOOST_AUTO_TEST_CASE(CommandModules)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager();
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  dunedaq::appfwk::app::Init init;
  dunedaq::appfwk::app::ModSpec module_init;
  module_init.inst = "DummyModule";
  module_init.plugin = "DummyModule";
  init.modules.push_back(module_init);
  nlohmann::json init_data;
  to_json(init_data, init);
  // dunedaq::cmdlib::cmd::Command cmd;
  // cmd.id = "init";
  // cmd.data = init_data;
  // nlohmann::json cmd_data;
  // to_json(cmd_data, cmd);
  mgr.initialize(init_data);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);
  nlohmann::json cmd_data;
  mgr.execute("RUNNING", "stuff", cmd_data);

  BOOST_REQUIRE_EXCEPTION(mgr.execute("RUNNING", "bad_stuff", cmd_data),
                          CommandDispatchingFailed,
                          [&](CommandDispatchingFailed) { return true; });

  dunedaq::opmonlib::InfoCollector ic;
  mgr.gather_stats(ic, 0);
}

BOOST_AUTO_TEST_CASE(CommandMatchingModules)
{
  dunedaq::get_iomanager()->reset();
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
  // cmd.id = "init";
  // cmd.data = init_data;
  // nlohmann::json cmd_data;
  // to_json(cmd_data, cmd);
  // mgr.execute("NONE", cmd_data);
  mgr.initialize(init_data);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);

  dunedaq::appfwk::cmd::CmdObj cmd_obj;
  nlohmann::json cmd_obj_data;
  dunedaq::appfwk::cmd::AddressedCmd addr_cmd;
  addr_cmd.match = "Dummy.*";
  cmd_obj.modules.push_back(addr_cmd);
  addr_cmd.match = "foo";
  cmd_obj.modules.push_back(addr_cmd);
  to_json(cmd_obj_data, cmd_obj);
  mgr.execute("RUNNING", "stuff", cmd_obj_data);

  addr_cmd.match = ".*Module";
  cmd_obj.modules.push_back(addr_cmd);
  to_json(cmd_obj_data, cmd_obj);
  BOOST_REQUIRE_EXCEPTION(mgr.execute("RUNNING", "bad_stuff", cmd_obj_data),
                          ConflictingCommandMatching,
                          [&](ConflictingCommandMatching) { return true; });
}

BOOST_AUTO_TEST_CASE(InitializeIOManager_Queues)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager();
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  dunedaq::appfwk::app::Init init;
  dunedaq::iomanager::connection::QueueConfig queue_init;
  queue_init.id.uid = "test_deque";
  queue_init.queue_type = dunedaq::iomanager::connection::QueueType::kStdDeQueue;
  queue_init.capacity = 10;
  init.queues.push_back(queue_init);
  queue_init.id.uid = "test_spsc";
  queue_init.queue_type = dunedaq::iomanager::connection::QueueType::kFollySPSCQueue;
  init.queues.push_back(queue_init);
  queue_init.id.uid = "test_mpmc";
  queue_init.queue_type = dunedaq::iomanager::connection::QueueType::kFollyMPMCQueue;
  init.queues.push_back(queue_init);
  nlohmann::json init_data;
  to_json(init_data, init);
  mgr.initialize(init_data);
  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);
}

BOOST_AUTO_TEST_CASE(InitializeIOManager_Network)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager();
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  dunedaq::appfwk::app::Init init;
  dunedaq::iomanager::connection::Connection conn_init;
  conn_init.id.uid = "test_inproc";
  conn_init.connection_type = dunedaq::iomanager::connection::ConnectionType::kSendRecv;
  conn_init.uri = "inproc://foo";
  init.connections.push_back(conn_init);
  conn_init.id.uid = "test_tcp";
  conn_init.uri = "tcp://localhost:1234";
  init.connections.push_back(conn_init);
  nlohmann::json init_data;
  to_json(init_data, init);
  mgr.initialize(init_data);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);
}

BOOST_AUTO_TEST_CASE(InitializeIOManager_QueuesAndNetwork)
{
  dunedaq::get_iomanager()->reset();
  auto mgr = DAQModuleManager();
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  dunedaq::appfwk::app::Init init;
  dunedaq::iomanager::connection::QueueConfig queue_init;
  queue_init.id.uid = "test_deque";
  queue_init.queue_type = dunedaq::iomanager::connection::QueueType::kStdDeQueue;
  queue_init.capacity = 10;
  init.queues.push_back(queue_init);
  queue_init.id.uid = "test_spsc";
  queue_init.queue_type = dunedaq::iomanager::connection::QueueType::kFollySPSCQueue;
  init.queues.push_back(queue_init);
  queue_init.id.uid = "test_mpmc";
  queue_init.queue_type = dunedaq::iomanager::connection::QueueType::kFollyMPMCQueue;
  init.queues.push_back(queue_init);

  dunedaq::iomanager::connection::Connection conn_init;
  conn_init.id.uid = "test_inproc";
  conn_init.connection_type = dunedaq::iomanager::connection::ConnectionType::kSendRecv;
  conn_init.uri = "inproc://foo";
  init.connections.push_back(conn_init);
  conn_init.id.uid = "test_tcp";
  conn_init.uri = "tcp://localhost:1234";
  init.connections.push_back(conn_init);
  nlohmann::json init_data;
  to_json(init_data, init);
  mgr.initialize(init_data);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);
}

BOOST_AUTO_TEST_SUITE_END()
