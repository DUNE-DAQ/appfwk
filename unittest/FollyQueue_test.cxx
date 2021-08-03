/**
 *
 * @file FollyQueue_test.cxx FollyQueue class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/FollyQueue.hpp"

#define BOOST_TEST_MODULE FollyQueue_test // NOLINT
#include "boost/test/included/unit_test.hpp"

#include <chrono>
#include <utility>

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
constexpr auto timeout = std::chrono::milliseconds(1);
/**
 * @brief Timeout expressed in microseconds
 */
constexpr auto timeout_in_us = std::chrono::duration_cast<std::chrono::microseconds>(timeout).count();

dunedaq::appfwk::FollySPSCQueue<int> queue("FollyQueue", 10); ///< Queue instance for the test

} // namespace ""

// This test case should run first. Make sure all other test cases depend on
// this.

BOOST_AUTO_TEST_CASE(sanity_checks)
{
  BOOST_REQUIRE(!queue.can_pop());

  BOOST_REQUIRE_EQUAL(queue.get_capacity(), 10);
  BOOST_REQUIRE_EQUAL(queue.get_num_elements(), 0);

  auto start_time = std::chrono::steady_clock::now();
  try {
    BOOST_REQUIRE(queue.can_push());
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
                       "resulted in a timeout (function exited after "
                         << push_time_in_us << " microseconds, timeout is " << timeout_in_us << " microseconds)");
  }

  BOOST_REQUIRE(queue.can_pop());
  BOOST_REQUIRE_EQUAL(queue.get_num_elements(), 1);

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

BOOST_AUTO_TEST_CASE(empty_checks, *boost::unit_test::depends_on("sanity_checks"))
{
  int popped_value = -999;

  while (queue.can_pop()) {

    try {
      queue.pop(popped_value, timeout);
    } catch (const dunedaq::appfwk::QueueTimeoutExpired& ex) {
      BOOST_TEST(false,
                 "Timeout exception thrown in call to FollyQueue::pop(); unable "
                 "to empty the Queue");
      break;
    }
  }

  BOOST_REQUIRE(!queue.can_pop());

  // pop off of an empty Queue

  auto start_time = std::chrono::steady_clock::now();
  BOOST_CHECK_THROW(queue.pop(popped_value, timeout), dunedaq::appfwk::QueueTimeoutExpired);
  auto pop_duration = std::chrono::steady_clock::now() - start_time;

  const double fraction_of_pop_timeout_used =
    static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(pop_duration).count()) /
    std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count();

  BOOST_TEST_MESSAGE("Attempted pop_duration divided by timeout is " << fraction_of_pop_timeout_used);

  BOOST_CHECK_GT(fraction_of_pop_timeout_used, 1 - fractional_timeout_tolerance);
  BOOST_CHECK_LT(fraction_of_pop_timeout_used, 1 + fractional_timeout_tolerance);
}

BOOST_AUTO_TEST_CASE(full_checks, *boost::unit_test::depends_on("empty_checks"))
{
  int push_value = 0;

  while (queue.can_push()) {

    try {
      int push_value_tmp = push_value;
      queue.push(std::move(push_value_tmp), timeout);
      push_value++;
    } catch (const dunedaq::appfwk::QueueTimeoutExpired& ex) {
      BOOST_TEST(false,
                 "Timeout exception thrown in call to FollyQueue::push(); unable "
                 "to fill the Queue");
      break;
    }
  }

  BOOST_REQUIRE(!queue.can_push());
  BOOST_REQUIRE_EQUAL(push_value, queue.get_capacity());

  int test_max_capacity = 1000000;
  while (push_value < test_max_capacity) {
    // push to a full Queue
    auto start_time = std::chrono::steady_clock::now();
    try {
      int push_value_tmp = push_value;
      queue.push(std::move(push_value_tmp), timeout);
      push_value++;
    } catch (dunedaq::appfwk::QueueTimeoutExpired&) {
      auto push_duration = std::chrono::steady_clock::now() - start_time;
      BOOST_TEST_MESSAGE("Timeout occurred. Capacity is " << queue.get_capacity() << ", current occupancy is "
                                                          << queue.get_num_elements());

      const double fraction_of_push_timeout_used =
        static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(push_duration).count()) /
        std::chrono::duration_cast<std::chrono::nanoseconds>(timeout).count();

      BOOST_TEST_MESSAGE("Attempted push_duration divided by timeout is " << fraction_of_push_timeout_used);

      BOOST_CHECK_GT(fraction_of_push_timeout_used, 1 - fractional_timeout_tolerance);
      BOOST_CHECK_LT(fraction_of_push_timeout_used, 1 + fractional_timeout_tolerance);
      break;
    }
  }
  if (push_value == test_max_capacity) {
    BOOST_TEST_MESSAGE("Unable to cause push timeout in " << test_max_capacity << " pushes");
  }
}
