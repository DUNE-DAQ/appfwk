/**
 *
 * @file StdDeQueue class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "app-framework/Queues/StdDeQueue.hh"

#define BOOST_TEST_MODULE StdDeQueue_test // NOLINT
#include <boost/test/included/unit_test.hpp>

#include <chrono>

// For a first look at the code, you may want to skip past the
// contents of the unnamed namespace and move ahead to the actual test
// cases

namespace {

constexpr int max_testable_capacity = 1000000000;
constexpr double fractional_timeout_tolerance = 0.1;

// Don't set the timeout to zero, otherwise the tests will fail since they'd
// expect the push/pop functions to execute instananeously
constexpr auto timeout = std::chrono::milliseconds(1);
constexpr auto timeout_in_us =
  std::chrono::duration_cast<std::chrono::microseconds>(timeout).count();

appframework::StdDeQueue<int> Queue("StdDeQueue");

/**
 * \todo StdDeQueue no longer exposes size or capacity methods. This section
 * should either be deleted or those methods added back
 *
 * ELF, May 19, 2020
 */
#if 0
// See
// https://www.boost.org/doc/libs/1_73_0/libs/test/doc/html/boost_test/tests_organization/enabling.html
// to better understand CapacityChecker.

struct CapacityChecker
{

  boost::test_tools::assertion_result operator()(boost::unit_test::test_unit_id)
  {
    if (Queue.capacity() <= max_testable_capacity) {
      return true;
    } else {
      boost::test_tools::assertion_result result(false);
      result.message() << "Capacity of StdDeQueue (" << Queue.capacity()
                       << ") larger than max value this suite tests ("
                       << max_testable_capacity << ")";
      return result;
    }
  }
};
#endif

} // namespace ""

// This test case should run first. Make sure all other test cases depend on
// this.

BOOST_AUTO_TEST_CASE(sanity_checks)
{
  Queue.SetSize(10);

  BOOST_REQUIRE(!Queue.can_pop());

  auto starttime = std::chrono::steady_clock::now();
  Queue.push(999, timeout);
  auto push_time = std::chrono::steady_clock::now() - starttime;

  if (push_time > timeout) {
    auto push_time_in_us =
      std::chrono::duration_cast<std::chrono::microseconds>(push_time).count();

    BOOST_TEST_REQUIRE(false,
                       "Test failure: pushing element onto empty Queue "
                       "resulted in a timeout (function exited after "
                         << push_time_in_us << " microseconds, timeout is "
                         << timeout_in_us << " microseconds)");
  }

  BOOST_REQUIRE(Queue.can_pop());

  starttime = std::chrono::steady_clock::now();
  auto popped_value = Queue.pop(timeout);
  auto pop_time = std::chrono::steady_clock::now() - starttime;

  if (pop_time > timeout) {
    auto pop_time_in_us =
      std::chrono::duration_cast<std::chrono::microseconds>(pop_time).count();
    BOOST_TEST_REQUIRE(false,
                       "Test failure: popping element off Queue "
                       "resulted in a timeout (function exited after "
                         << pop_time_in_us << " microseconds, timeout is "
                         << timeout_in_us << " microseconds)");
  }

  BOOST_REQUIRE_EQUAL(popped_value, 999);
}

BOOST_AUTO_TEST_CASE(empty_checks,
                     *boost::unit_test::depends_on("sanity_checks"))
{

  try {
    while (Queue.can_pop()) {
      Queue.pop(timeout);
    }
  } catch (const std::runtime_error& err) {
    BOOST_WARN_MESSAGE(true, err.what());
    BOOST_TEST(false,
               "Exception thrown in call to StdDeQueue::pop(); unable "
               "to empty the Queue");
  }

  BOOST_REQUIRE(!Queue.can_pop());

  // pop off of an empty Queue

  auto starttime = std::chrono::steady_clock::now();
  BOOST_CHECK_THROW(Queue.pop(timeout), std::runtime_error);
  auto pop_duration = std::chrono::steady_clock::now() - starttime;

  const double fraction_of_pop_timeout_used = pop_duration / timeout;

  BOOST_CHECK_GT(fraction_of_pop_timeout_used,
                 1 - fractional_timeout_tolerance);
  BOOST_CHECK_LT(fraction_of_pop_timeout_used,
                 1 + fractional_timeout_tolerance);
}

/**
 * \todo StdDeQueue no longer exposes size or capacity methods. This section
 * should either be deleted or those methods added back
 *
 * ELF, May 19, 2020
 */
#if 0
BOOST_AUTO_TEST_CASE(capacity_checks,
                   *boost::unit_test::precondition(CapacityChecker()) *
                       boost::unit_test::depends_on("sanity_checks")) {

// TODO, May-6-2020, John Freeman (jcfree@fnal.gov)
// In the next week, figure out if it makes sense beyond this test to create
// an insert() function which takes iterators
try {
  while (Queue.size() < Queue.capacity()) {
    Queue.push(-1, timeout);
  }
} catch (const std::runtime_error &err) {
  BOOST_WARN_MESSAGE(true, err.what());
  BOOST_TEST(false, "Exception thrown in call to StdDeQueue::push(); unable "
                    "to fill the Queue to its alleged capacity of "
                        << Queue.capacity() << " elements");
}
BOOST_REQUIRE(!Queue.can_push());

// Push onto an already-full Queue

auto starttime = std::chrono::steady_clock::now();
try {
  Queue.push(-1, timeout);
} catch (...) { // NOLINT (don't care whether or not a failed push results in
                // an exception)
}
auto push_duration = std::chrono::steady_clock::now() - starttime;

// Trying to push an element onto a Queue at capacity shouldn't change its
// size

BOOST_REQUIRE(!Queue.can_push());

const double fraction_of_push_timeout_used = push_duration / timeout;

BOOST_CHECK_GT(fraction_of_push_timeout_used,
               1 - fractional_timeout_tolerance);
BOOST_CHECK_LT(fraction_of_push_timeout_used,
               1 + fractional_timeout_tolerance);
}
#endif
