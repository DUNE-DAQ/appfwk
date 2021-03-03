/**
 * @file Interruptible_test.cxx Interruptible class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/Interruptible.hpp"

#include "logging/Logging.hpp"

#define BOOST_TEST_MODULE Interruptible_test // NOLINT

#include "boost/test/unit_test.hpp"

#include <chrono>
#include <thread>

constexpr auto queue_timeout = std::chrono::milliseconds(10);
using namespace dunedaq::appfwk;

BOOST_AUTO_TEST_SUITE(Interruptible_test)

namespace interruptibletest {
class TestInterruptible : public Interruptible
{
public:
  explicit TestInterruptible(bool initial_condition = false)
    : m_wait_condition(initial_condition)
  {}

  void set_end_wait_condition() { m_wait_condition = true; }

  void interrupt() override
  {
    set_end_wait_condition();
    interrupt_self();
  }

  bool wait_proc(int sleep_time_ms)
  {
    auto start_time = std::chrono::steady_clock::now();
    auto res = interruptible_wait(std::chrono::microseconds(sleep_time_ms * 1000), m_wait_condition);
    m_wait_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start_time);

    return res;
  }

  std::chrono::milliseconds m_wait_time;
  std::atomic<bool> m_wait_condition;
};

} // namespace interruptibletest

BOOST_AUTO_TEST_CASE(Construct)
{
  interruptibletest::TestInterruptible ti;
}

BOOST_AUTO_TEST_CASE(FullWait)
{
  interruptibletest::TestInterruptible ti;
  auto res = ti.wait_proc(10);

  BOOST_REQUIRE_EQUAL(res, false);
  BOOST_REQUIRE(ti.m_wait_time >= std::chrono::milliseconds(10));
  TLOG() << "Wait time was " << ti.m_wait_time.count() << " ms";
}

BOOST_AUTO_TEST_CASE(NullWait)
{
  interruptibletest::TestInterruptible ti(true);
  auto res = ti.wait_proc(10);

  BOOST_REQUIRE_EQUAL(res, true);
  BOOST_REQUIRE(ti.m_wait_time < std::chrono::milliseconds(10));
  TLOG() << "Wait time was " << ti.m_wait_time.count() << " ms";
}

BOOST_AUTO_TEST_CASE(WaitThread) 
{
  interruptibletest::TestInterruptible ti;
  auto wait_thread = std::thread([&]() { ti.wait_proc(10); });
  wait_thread.join();
  BOOST_REQUIRE(ti.m_wait_time >= std::chrono::milliseconds(10));
  TLOG() << "Wait time was " << ti.m_wait_time.count() << " ms";
}

BOOST_AUTO_TEST_CASE(Interrupt) {
  interruptibletest::TestInterruptible ti;
  auto wait_thread = std::thread([&]() { ti.wait_proc(10); });
  usleep(5000);
  ti.interrupt();
  wait_thread.join();
  BOOST_REQUIRE(ti.m_wait_time < std::chrono::milliseconds(10));
  TLOG() << "Wait time was " << ti.m_wait_time.count() << " ms";
}


BOOST_AUTO_TEST_SUITE_END()
