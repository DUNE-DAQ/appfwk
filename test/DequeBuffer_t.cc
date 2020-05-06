
#define BOOST_TEST_MODULE DequeBuffer_t
#include <boost/test/included/unit_test.hpp>

#include "app-framework/Buffers/DequeBuffer.hh"

appframework::DequeBuffer<int> buffer;

BOOST_AUTO_TEST_CASE(sanity_checks) {

  BOOST_REQUIRE_EQUAL(buffer.size(), 0);
  BOOST_REQUIRE(buffer.empty());

  buffer.push(999);
  BOOST_REQUIRE_EQUAL(buffer.size(), 1);

  auto popped_value = buffer.pop();
  BOOST_REQUIRE_EQUAL(popped_value, 999);

}
