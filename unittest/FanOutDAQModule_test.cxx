/**
 * @file FanOutDAQModule_test.cxx FanOutDAQModule class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/FanOutDAQModule.hpp"
#include "appfwk/QueueRegistry.hpp"

#define BOOST_TEST_MODULE FanOutDAQModule_test // NOLINT

#include <boost/test/unit_test.hpp>
#include <nlohmann/json.hpp>

constexpr auto queue_timeout = std::chrono::milliseconds(10);
using namespace dunedaq::appfwk;

BOOST_AUTO_TEST_SUITE(FanOutDAQModule_test)

/**
 * @brief Initializes the QueueRegistry for use by the FanOutDAQModule test
 */
struct FanOutDAQModuleTestFixture
{
  FanOutDAQModuleTestFixture() {}
  void setup()
  {

    std::map<std::string, QueueConfig> queue_config;
    queue_config["input"].kind = QueueConfig::queue_kind::kStdDeQueue;
    queue_config["input"].size = 10;
    queue_config["output1"].kind = QueueConfig::queue_kind::kStdDeQueue;
    queue_config["output1"].size = 5;
    queue_config["output2"].kind = QueueConfig::queue_kind::kStdDeQueue;
    queue_config["output2"].size = 5;

    QueueRegistry::get().configure(queue_config);
  }
};

BOOST_TEST_GLOBAL_FIXTURE(FanOutDAQModuleTestFixture);

BOOST_AUTO_TEST_CASE(Construct)
{
  dunedaq::appfwk::FanOutDAQModule<int> foum("test");
}

BOOST_AUTO_TEST_CASE(Configure)
{
  dunedaq::appfwk::FanOutDAQModule<dunedaq::appfwk::NonCopyableType> foum("test");

  auto config = R"({"input": "input"})"_json;
  foum.configure(config);

  foum.execute_command("configure");
}

BOOST_AUTO_TEST_CASE(NonCopyableTypeTest)
{
  dunedaq::appfwk::FanOutDAQModule<dunedaq::appfwk::NonCopyableType> foum("test");

  nlohmann::json module_config = R"(
        {
                    "input": "input",
                    "outputs": ["output1", "output2" ],
                    "fanout_mode": "RoundRobin"
        }
    )"_json;
  foum.configure(module_config);

  // This test assumes RoundRobin mode. Once configurability is implemented,
  // we'll have to configure it appropriately.
  foum.execute_command("configure");
  foum.execute_command("start");

  DAQSink<NonCopyableType> inputbuf("input");
  DAQSource<NonCopyableType> outputbuf1("output1");
  DAQSource<NonCopyableType> outputbuf2("output2");

  inputbuf.push(dunedaq::appfwk::NonCopyableType(1), queue_timeout);
  inputbuf.push(dunedaq::appfwk::NonCopyableType(2), queue_timeout);

  std::this_thread::sleep_for(std::chrono::milliseconds(10));

  foum.execute_command("stop");

  BOOST_REQUIRE_EQUAL(outputbuf1.can_pop(), true);
  dunedaq::appfwk::NonCopyableType res(0);
  outputbuf1.pop(res, queue_timeout);
  BOOST_REQUIRE_EQUAL(res.data, 1);

  BOOST_REQUIRE_EQUAL(outputbuf2.can_pop(), true);
  outputbuf2.pop(res, queue_timeout);
  BOOST_REQUIRE_EQUAL(res.data, 2);
}

BOOST_AUTO_TEST_SUITE_END()
