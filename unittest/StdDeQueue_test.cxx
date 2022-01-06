/**
 *
 * @file StdDeQueue_test.cxx StdDeQueue class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/StdDeQueue.hpp"

#define BOOST_TEST_MODULE StdDeQueue_test // NOLINT
#include "boost/test/included/unit_test.hpp"

#include <chrono>

BOOST_AUTO_TEST_SUITE(StdDeQueue_test)

// For a first look at the code, you may want to skip past the
// contents of the unnamed namespace and move ahead to the actual test
// cases

namespace {

constexpr int max_testable_capacity = 1000000000; ///< The maximum capacity this test will attempt to check

constexpr double fractional_timeout_tolerance =
  0.5; ///< The fraction of the timeout which the timing is allowed to be off by

/**
 * @brief Timeout to use for tests
 *
 * Don't set the timeout to zero, otherwise the tests will fail since they'd
 * expect the push/pop functions to execute instananeously
 */
constexpr auto timeout = std::chrono::milliseconds(5);
/**
 * @brief Timeout expressed in microseconds
 */
constexpr auto timeout_in_us = std::chrono::duration_cast<std::chrono::microseconds>(timeout).count();

dunedaq::appfwk::StdDeQueue<int> queue("StdDeQueue", 10); ///< Queue instance for the test
} // namespace ""

// This test case should run first. Make sure all other test cases depend on
// this.

BOOST_AUTO_TEST_CASE(sanity_checks)
{

  BOOST_REQUIRE(!queue.can_pop());

  auto start_time = std::chrono::steady_clock::now();
  try {
    queue.push(42, timeout);
  } catch (const dunedaq::appfwk::QueueTimeoutExpired& ex) {
    BOOST_TEST_REQUIRE(false, "Test failure: unexpected timeout exception throw from push");
  } catch (...) { // NOLINT
    BOOST_TEST_REQUIRE(false, "Test failure: unexpected exception (non-timeout-related) thrown");
  }

  auto push_time = std::chrono::steady_clock::now() - start_time;

  if (push_time > timeout) {
    auto push_time_in_us = std::chrono::duration_cast<std::chrono::microseconds>(push_time).count();

    BOOST_TEST_REQUIRE(false,
                       "Test failure: pushing element onto empty Queue "
                       "resulted in a timeout without an exception throw (function exited after "
                         << push_time_in_us << " microseconds, timeout is " << timeout_in_us << " microseconds)");
  }

  BOOST_REQUIRE(queue.can_pop());

  start_time = std::chrono::steady_clock::now();
  int popped_value = -999;
  try {
    queue.pop(popped_value, timeout);
  } catch (const dunedaq::appfwk::QueueTimeoutExpired& ex) {
    BOOST_TEST_REQUIRE(false, "Test failure: unexpected timeout exception throw from pop");
  } catch (...) { // NOLINT
    BOOST_TEST_REQUIRE(false, "Test failure: unexpected exception (non-timeout-related) thrown");
  }
  auto pop_time = std::chrono::steady_clock::now() - start_time;

  if (pop_time > timeout) {
    auto pop_time_in_us = std::chrono::duration_cast<std::chrono::microseconds>(pop_time).count();
    BOOST_TEST_REQUIRE(false,
                       "Test failure: popping element off Queue "
                       "resulted in a timeout without an exception throw (function exited after "
                         << pop_time_in_us << " microseconds, timeout is " << timeout_in_us << " microseconds)");
  }

  BOOST_REQUIRE_EQUAL(popped_value, 42);
}

BOOST_AUTO_TEST_CASE(empty_checks)
{
  int popped_value = -999;

  while (queue.can_pop()) {

    try {
      queue.pop(popped_value, timeout);
    } catch (const dunedaq::appfwk::QueueTimeoutExpired& ex) {
      BOOST_TEST(false,
                 "Timeout exception thrown in call to StdDeQueue::pop(); unable "
                 "to empty the Queue");
      break;
    }
  }

  BOOST_REQUIRE(!queue.can_pop());

  // pop off of an empty Queue

  auto starttime = std::chrono::steady_clock::now();

  BOOST_CHECK_THROW(queue.pop(popped_value, timeout), dunedaq::appfwk::QueueTimeoutExpired);

  auto pop_duration = std::chrono::steady_clock::now() - starttime;

  const double fraction_of_pop_timeout_used =
    static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(pop_duration).count()) /
    std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count();

  BOOST_TEST_MESSAGE("Attempted pop_duration divided by timeout is " << fraction_of_pop_timeout_used);

  BOOST_CHECK_GT(fraction_of_pop_timeout_used, 1 - fractional_timeout_tolerance);
  BOOST_CHECK_LT(fraction_of_pop_timeout_used, 1 + fractional_timeout_tolerance);
}

BOOST_AUTO_TEST_SUITE_END()
