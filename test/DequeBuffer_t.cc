
#include "app-framework/Buffers/DequeBuffer.hh"

#define BOOST_TEST_MODULE DequeBuffer_t
#include <boost/test/included/unit_test.hpp>

#include <chrono>

// For a first look at the code, you may want to skip past the
// contents of the unnamed namespace and move ahead to the actual test
// cases

namespace {
  appframework::DequeBuffer<int> buffer;

  constexpr int max_testable_capacity = 1000000000;
  constexpr double fractional_timeout_tolerance = 0.1;

// See
// https://www.boost.org/doc/libs/1_73_0/libs/test/doc/html/boost_test/tests_organization/enabling.html
// to better understand CapacityChecker

struct CapacityChecker {

  boost::test_tools::assertion_result operator()(boost::unit_test::test_unit_id) {
    if (buffer.capacity() <= max_testable_capacity) {
      return true;
    } else {
      boost::test_tools::assertion_result result(false);
      result.message() << "Capacity of DequeBuffer (" << buffer.capacity() << ") larger than max value this suite tests (" << max_testable_capacity << ")";
      return result;
    }
}
};

} // namespace ""

BOOST_AUTO_TEST_CASE(sanity_checks) {

  BOOST_REQUIRE_EQUAL(buffer.size(), 0);
  BOOST_REQUIRE(buffer.empty());

  buffer.push(999);
  BOOST_REQUIRE_EQUAL(buffer.size(), 1);

  auto popped_value = buffer.pop();
  BOOST_REQUIRE_EQUAL(popped_value, 999);

}

BOOST_AUTO_TEST_CASE(capacity_checks, * boost::unit_test::precondition(CapacityChecker()))
{
 
  // TODO, May-6-2020, John Freeman (jcfree@fnal.gov)
  // In the next week, figure out if it makes sense beyond this test to create an insert() function which takes iterators

  while (buffer.size() < buffer.capacity()) {
    buffer.push(-1);
  }

  BOOST_REQUIRE( buffer.full() );

  auto starttime = std::chrono::steady_clock::now();
  try {
    buffer.push(-1);
  } catch (...) { // NO LINT (We legitimately don't care about any exceptions since we're testing)
  }
  auto push_duration = std::chrono::steady_clock::now() - starttime;

  // Trying to push an element onto a buffer at capacity shouldn't change its size
  BOOST_REQUIRE_EQUAL(buffer.size(), buffer.capacity());

  const double push_timeout_in_milliseconds = 1000;
  const double fraction_of_timeout_used = std::chrono::duration_cast<std::chrono::milliseconds>(push_duration).count() / push_timeout_in_milliseconds;

  BOOST_REQUIRE_GT(fraction_of_timeout_used, 1 - fractional_timeout_tolerance);
  BOOST_REQUIRE_LT(fraction_of_timeout_used, 1 + fractional_timeout_tolerance);

}
