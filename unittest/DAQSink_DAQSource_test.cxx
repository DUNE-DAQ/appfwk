/**
 * @file DAQSink_DAQSource_test.cxx Test application which demonstrates the
 * functionality of the QueueRegistry, DAQSink, and DAQSource
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/DAQSink.hpp"
#include "appfwk/DAQSource.hpp"
#include "appfwk/StdDeQueue.hpp"

#include "ers/ers.h"

#define BOOST_TEST_MODULE DAQSink_DAQSource_test // NOLINT

#include <boost/test/unit_test.hpp>

using namespace dunedaq::appfwk;

BOOST_AUTO_TEST_SUITE(DAQSink_DAQSource_test)

/**
 * @brief Initializes the QueueRegistry
 */
struct DAQSinkDAQSourceTestFixture
{
  DAQSinkDAQSourceTestFixture() {}

  void setup()
  {
    std::map<std::string, QueueConfig> queuemap = { { "dummy", { QueueConfig::queue_kind::kStdDeQueue, 100 } } };

    QueueRegistry::get().configure(queuemap);
  }
};

BOOST_TEST_GLOBAL_FIXTURE(DAQSinkDAQSourceTestFixture);

BOOST_AUTO_TEST_CASE(Construct)
{
  DAQSink<std::string> sink("dummy");
  DAQSource<std::string> source("dummy");

  [[maybe_unused]] DAQSink<int>* badsink;

  BOOST_REQUIRE_EXCEPTION(badsink = new DAQSink<int>("dummy"),
                          dunedaq::appfwk::DAQSinkConstructionFailed,
                          [&](dunedaq::appfwk::DAQSinkConstructionFailed) { return true; });


  [[maybe_unused]] DAQSource<int>* badsource;

  BOOST_REQUIRE_EXCEPTION(badsource = new DAQSource<int>("dummy"),
                          dunedaq::appfwk::DAQSourceConstructionFailed,
                          [&](dunedaq::appfwk::DAQSourceConstructionFailed) { return true; });

}

BOOST_AUTO_TEST_CASE(InitialConditions)
{

  DAQSink<std::string> sink("dummy");
  DAQSource<std::string> source("dummy");

  BOOST_REQUIRE(sink.can_push());
  BOOST_REQUIRE(!source.can_pop());
}

BOOST_AUTO_TEST_CASE(DataFlow)
{

  DAQSink<std::string> sink("dummy");
  DAQSource<std::string> source("dummy");

  sink.push(std::move("hello"));
  std::string res;
  source.pop(res);
  BOOST_REQUIRE_EQUAL(res, "hello");
}

BOOST_AUTO_TEST_CASE(Exceptions)
{

  DAQSink<std::string> sink("dummy");
  DAQSource<std::string> source("dummy");
  std::string res;

  BOOST_REQUIRE(!source.can_pop());
  BOOST_REQUIRE(!source.pop(res));

  for (int ii = 0; ii < 100; ++ii) {
    BOOST_REQUIRE(sink.can_push());
    sink.push("aaAaaa");
  }

  BOOST_REQUIRE(!sink.can_push());
  BOOST_REQUIRE_EXCEPTION(    sink.push("bbBbbb"), dunedaq::appfwk::DequeTimeoutExpired, [&](dunedaq::appfwk::DequeTimeoutExpired) {
    return true;
  }); // TODO: Improve check
}

BOOST_AUTO_TEST_SUITE_END()
