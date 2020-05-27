/**
 * @file FanOutDAQModule class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "app-framework-base/Services/CommandFacility.hh"
#include "app-framework/DAQModules/FanOutDAQModule.hh"
#include "app-framework/Queues/StdDeQueue.hh"

#define BOOST_TEST_MODULE FanOutDAQModule_test // NOLINT

#include <boost/test/unit_test.hpp>

namespace {

constexpr auto buffer_timeout = std::chrono::milliseconds(10);

struct NonCopyableType
{
  int data;
  explicit NonCopyableType(int d)
    : data(d)
  {}
  NonCopyableType(NonCopyableType const&) = delete;
  NonCopyableType(NonCopyableType&& i) { data = i.data; }
  NonCopyableType& operator=(NonCopyableType const&) = delete;
  NonCopyableType& operator=(NonCopyableType&& i)
  {
    data = i.data;
    return *this;
  }
};

} // namespace ""

namespace appframework {
std::unique_ptr<CommandFacility> CommandFacility::handle_ = nullptr;
} // namespace appframework

BOOST_AUTO_TEST_SUITE(FanOutDAQModule_test)

BOOST_AUTO_TEST_CASE(Construct)
{
  auto buf = std::make_shared<appframework::StdDeQueue<int>>();
  appframework::FanOutDAQModule<int> foum(buf, { buf });
}

BOOST_AUTO_TEST_CASE(Configure)
{
  auto buf = std::make_shared<appframework::StdDeQueue<int>>();
  appframework::FanOutDAQModule<int> foum(buf, { buf });
  foum.execute_command("configure");
}

BOOST_AUTO_TEST_CASE(NonCopyableTypeTest)
{
  auto inputbuf = std::make_shared<appframework::StdDeQueue<NonCopyableType>>();
  auto outputbuf1 =
    std::make_shared<appframework::StdDeQueue<NonCopyableType>>();
  auto outputbuf2 =
    std::make_shared<appframework::StdDeQueue<NonCopyableType>>();
  appframework::FanOutDAQModule<NonCopyableType> foum(
    inputbuf, { outputbuf1, outputbuf2 });

  // This test assumes RoundRobin mode. Once configurability is implemented,
  // we'll have to configure it appropriately.
  foum.execute_command("configure");
  foum.execute_command("start");

  inputbuf->push(NonCopyableType(1), buffer_timeout);
  inputbuf->push(NonCopyableType(2), buffer_timeout);

  while (inputbuf->can_pop())
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

  foum.execute_command("stop");

  BOOST_REQUIRE_EQUAL(inputbuf->can_pop(), false);

  BOOST_REQUIRE_EQUAL(outputbuf1->can_pop(), true);
  auto res = outputbuf1->pop(buffer_timeout);
  BOOST_REQUIRE_EQUAL(res.data, 1);

  BOOST_REQUIRE_EQUAL(outputbuf2->can_pop(), true);
  res = outputbuf2->pop(buffer_timeout);
  BOOST_REQUIRE_EQUAL(res.data, 2);
}

BOOST_AUTO_TEST_SUITE_END()
