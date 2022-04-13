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
#include "iomanager/StdDeQueue.hpp"
#include "iomanager/QueueRegistry.hpp"

#include "ers/ers.hpp"

#define BOOST_TEST_MODULE DAQSink_DAQSource_test // NOLINT

#include "boost/test/unit_test.hpp"

#include <map>
#include <string>
#include <utility>
#include <future>
#include <thread>

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
    std::map<std::string, dunedaq::iomanager::QueueConfig> queue_map = {
      { "dummy", { dunedaq::iomanager::QueueConfig::queue_kind::kStdDeQueue, 100 } },
      { "short", { dunedaq::iomanager::QueueConfig::queue_kind::kStdDeQueue, 1 } }
    };

    dunedaq::iomanager::QueueRegistry::get().configure(queue_map);
  }
};

BOOST_TEST_GLOBAL_FIXTURE(DAQSinkDAQSourceTestFixture);

BOOST_AUTO_TEST_CASE(Construct)
{
  DAQSink<std::string> sink("dummy");
  DAQSource<std::string> source("dummy");

  [[maybe_unused]] DAQSink<int>* bad_sink;

  BOOST_REQUIRE_EXCEPTION(bad_sink = new DAQSink<int>("dummy"),
                          dunedaq::appfwk::DAQSinkConstructionFailed,
                          [&](dunedaq::appfwk::DAQSinkConstructionFailed) { return true; });

  [[maybe_unused]] DAQSource<int>* bad_source;

  BOOST_REQUIRE_EXCEPTION(bad_source = new DAQSource<int>("dummy"),
                          dunedaq::appfwk::DAQSourceConstructionFailed,
                          [&](dunedaq::appfwk::DAQSourceConstructionFailed) { return true; });
}

BOOST_AUTO_TEST_CASE(InitialConditions)
{

  DAQSink<std::string> sink("dummy");
  DAQSource<std::string> source("dummy");

  BOOST_REQUIRE_EQUAL(sink.get_name(), "dummy");
  BOOST_REQUIRE_EQUAL(source.get_name(), "dummy");

  BOOST_REQUIRE(sink.can_push());
  BOOST_REQUIRE(!source.can_pop());
}

BOOST_AUTO_TEST_CASE(DataFlow)
{

  DAQSink<std::string> sink("dummy");
  DAQSource<std::string> source("dummy");

  sink.push(std::move("hello"));
  std::string res("undefined");
  try {
    source.pop(res);
  } catch (const dunedaq::iomanager::QueueTimeoutExpired& ex) {
  }
  BOOST_REQUIRE_EQUAL(res, "hello");

  std::string test2 = "hello again";
  sink.push(test2);
  try {
    source.pop(res);
  } catch (const dunedaq::iomanager::QueueTimeoutExpired& ex) {
  }
  BOOST_REQUIRE_EQUAL(res, "hello again");
}

BOOST_AUTO_TEST_CASE(Exceptions)
{

  DAQSink<std::string> sink("dummy");
  DAQSource<std::string> source("dummy");
  std::string res;

  BOOST_REQUIRE(!source.can_pop());
  BOOST_CHECK_THROW(source.pop(res), dunedaq::iomanager::QueueTimeoutExpired);

  for (int ii = 0; ii < 100; ++ii) {
    bool can_push = sink.can_push();
    if (!can_push)
      BOOST_REQUIRE(can_push);
    sink.push("aaAaaa");
  }

  BOOST_REQUIRE(!sink.can_push());
  BOOST_REQUIRE_EXCEPTION(sink.push("bbBbbb"),
                          dunedaq::iomanager::QueueTimeoutExpired,
                          [&](dunedaq::iomanager::QueueTimeoutExpired) { return true; });
}

BOOST_AUTO_TEST_CASE(ShortQueue)
{

  DAQSink<int> sink("short");
  DAQSource<int> source("short");

  int res = 0;

  BOOST_REQUIRE(!source.can_pop());
  BOOST_REQUIRE(sink.can_push());
  
  sink.push(3);

  BOOST_CHECK(!sink.can_push());

  source.pop(res);

  BOOST_REQUIRE(sink.can_push());
  BOOST_REQUIRE(!source.can_pop());

  
  auto future = std::async(std::launch::async, 
    			   [&](){source.pop(res, std::chrono::milliseconds(10));
				 source.pop(res, std::chrono::milliseconds(10)); });

  BOOST_CHECK_NO_THROW( sink.push(4, std::chrono::milliseconds(10)) );

  BOOST_CHECK_THROW( future.get(), dunedaq::iomanager::QueueTimeoutExpired);
  
  BOOST_CHECK_EQUAL(res, 4);
}


BOOST_AUTO_TEST_SUITE_END()
