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

#include <string>
#include <type_traits>

BOOST_AUTO_TEST_SUITE(QueueRegistry_test)

using namespace dunedaq::appfwk;

BOOST_AUTO_TEST_CASE(Configure)
{
  std::map<std::string, QueueConfig> test_map;
  QueueConfig qc;
  qc.kind = QueueConfig::kUnknown;
  qc.capacity = 10;
  test_map["test_queue_unknown"] = qc;
  qc.kind = QueueConfig::kStdDeQueue;
  qc.capacity = 10;
  test_map["test_queue_stddeque"] = qc;
  qc.kind = QueueConfig::kFollySPSCQueue;
  qc.capacity = 10;
  test_map["test_queue_fspsc"] = qc;
  qc.kind = QueueConfig::kFollyMPMCQueue;
  qc.capacity = 10;
  test_map["test_queue_fmpmc"] = qc;

  QueueRegistry::get().configure(test_map);

  BOOST_REQUIRE_EXCEPTION(
    QueueRegistry::get().configure(test_map), QueueRegistryConfigured, [&](QueueRegistryConfigured) { return true; });
}

BOOST_AUTO_TEST_CASE(StoQK)
{
  BOOST_REQUIRE_EQUAL(QueueConfig::stoqk("StdDeQueue"), QueueConfig::kStdDeQueue);
  BOOST_REQUIRE_EQUAL(QueueConfig::stoqk("FollySPSCQueue"), QueueConfig::kFollySPSCQueue);
  BOOST_REQUIRE_EQUAL(QueueConfig::stoqk("FollyMPMCQueue"), QueueConfig::kFollyMPMCQueue);
  BOOST_REQUIRE_EXCEPTION(QueueConfig::stoqk("blahblahblah"), QueueKindUnknown, [&](QueueKindUnknown) { return true; });
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
