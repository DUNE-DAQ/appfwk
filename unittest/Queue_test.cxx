/**
 * @file Queue_test.cxx Queue class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/Queue.hpp"

#define BOOST_TEST_MODULE Queue_test // NOLINT

#include "boost/test/unit_test.hpp"

#include <memory>
#include <string>
#include <type_traits>

BOOST_AUTO_TEST_SUITE(Queue_test)

using namespace dunedaq::appfwk;

namespace queuetest {
template<class T>
class TestQueue : public Queue<T>
{
public:
  explicit TestQueue(const std::string& name)
    : Queue<T>(name)
  {}

  void push(T&&, const std::chrono::milliseconds&) override {}
  void pop(T&, const std::chrono::milliseconds&) override {}
  size_t get_capacity() const override { return 1; }
  size_t get_num_elements() const override { return 0; }
};
} // namespace queuetest

BOOST_AUTO_TEST_CASE(QueueOperations)
{

  auto queue_ptr = std::shared_ptr<Queue<int>>(new queuetest::TestQueue<int>("test_queue"));
  BOOST_REQUIRE(queue_ptr != nullptr);
  BOOST_REQUIRE(queue_ptr->can_push());
  BOOST_REQUIRE(!queue_ptr->can_pop());
  queue_ptr->push(15, std::chrono::milliseconds(1));
  int pop_value;
  queue_ptr->pop(pop_value, std::chrono::milliseconds(1));
}

BOOST_AUTO_TEST_SUITE_END()
