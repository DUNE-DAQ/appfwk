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
  qc.kind = QueueConfig::kStdDeQueue;
  qc.capacity = 10;
  test_map["test_queue"] = qc;
  
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

  auto queue_ptr = QueueRegistry::get().get_queue<int>("test_queue");
  BOOST_REQUIRE(queue_ptr != nullptr);
  QueueRegistry::get().gather_stats(ic, 1);
  BOOST_REQUIRE(!ic.is_empty());
}

BOOST_AUTO_TEST_SUITE_END()
