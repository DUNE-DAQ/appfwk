/**
 * @file QueueRegistry_test.cxx QueueRegistry class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/QueueRegistry.hpp"

#define BOOST_TEST_MODULE QueueRegistry_test // NOLINT

#include "boost/test/unit_test.hpp"

#include <map>
#include <string>
#include <type_traits>

BOOST_AUTO_TEST_SUITE(QueueRegistry_test)

using namespace dunedaq::appfwk;

BOOST_AUTO_TEST_CASE(Configure)
{
  app::QueueSpecs test_config;
  app::QueueSpec qc;
  qc.kind = app::QueueKind::Unknown;
  qc.capacity = 10;
  qc.inst = "test_queue_unknown";
  test_config.push_back(qc);
  qc.kind = app::QueueKind::StdDeQueue;
  qc.capacity = 10;
  qc.inst = "test_queue_stddeque";
  test_config.push_back(qc);
  qc.kind = app::QueueKind::FollySPSCQueue;
  qc.capacity = 10;
  qc.inst = "test_queue_fspsc";
  test_config.push_back(qc);
  qc.kind = app::QueueKind::FollyMPMCQueue;
  qc.capacity = 10;
  qc.inst = "test_queue_fmpmc";
  test_config.push_back(qc);

  QueueRegistry::get().configure(test_config);

  BOOST_REQUIRE_EXCEPTION(QueueRegistry::get().configure(test_config),
                          QueueRegistryConfigured,
                          [&](QueueRegistryConfigured) { return true; });
}

BOOST_AUTO_TEST_CASE(StoQK)
{
  BOOST_REQUIRE(app::parse_QueueKind("StdDeQueue") == app::QueueKind::StdDeQueue);
  BOOST_REQUIRE(app::parse_QueueKind("FollySPSCQueue") == app::QueueKind::FollySPSCQueue);
  BOOST_REQUIRE(app::parse_QueueKind("FollyMPMCQueue") == app::QueueKind::FollyMPMCQueue);
  BOOST_REQUIRE(app::parse_QueueKind("blahblahblah") == app::QueueKind::Unknown);
}

BOOST_AUTO_TEST_CASE(GatherStats)
{
  dunedaq::opmonlib::InfoCollector ic;
  QueueRegistry::get().gather_stats(ic, 1);
  BOOST_REQUIRE(ic.is_empty());

  auto queue_ptr = QueueRegistry::get().get_queue<int>("test_queue_stddeque");
  BOOST_REQUIRE(queue_ptr != nullptr);
  QueueRegistry::get().gather_stats(ic, 1);
  BOOST_REQUIRE(!ic.is_empty());
}

BOOST_AUTO_TEST_CASE(CreateQueue)
{

  auto queue_ptr_deque = QueueRegistry::get().get_queue<int>("test_queue_stddeque");
  BOOST_REQUIRE(queue_ptr_deque != nullptr);
  auto queue_ptr_fspsc = QueueRegistry::get().get_queue<int>("test_queue_fspsc");
  BOOST_REQUIRE(queue_ptr_fspsc != nullptr);
  auto queue_ptr_fmpmc = QueueRegistry::get().get_queue<int>("test_queue_fmpmc");
  BOOST_REQUIRE(queue_ptr_fmpmc != nullptr);
  BOOST_REQUIRE_EXCEPTION(QueueRegistry::get().get_queue<int>("test_queue_unknown"),
                          QueueKindUnknown,
                          [&](QueueKindUnknown) { return true; });
}

BOOST_AUTO_TEST_SUITE_END()
