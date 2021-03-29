/**
 *
 * @file FollyQueue_metric_test.cxx FollyQueue class Unit Tests to check if size() can be called in a thread 
 * that is neither the producer nor the consumer
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/FollyQueue.hpp"

#define BOOST_TEST_MODULE FollyQueue_metric_test // NOLINT
#include "boost/test/included/unit_test.hpp"

#include <chrono>
#include <future> 
#include <thread>

// For a first look at the code, you may want to skip past the
// contents of the unnamed namespace and move ahead to the actual test
// cases

namespace {

  constexpr int initial_size = 100 ; ///< the initial size of the queue ;

  constexpr auto test_time = std::chrono::milliseconds(500);

  constexpr auto timeout = std::chrono::milliseconds(1);

/**
 * @brief Timeout to use for tests
 *
 * Don't set the timeout to zero, otherwise the tests will fail since they'd
 * expect the push/pop functions to execute instananeously
 */

  folly::DSPSCQueue<int, true> queue(initial_size) ;   ///< Queue instance for the test

} // namespace ""

// This test case should run first. Make sure all other test cases depend on
// this.

BOOST_AUTO_TEST_CASE(three_thread_test)
{


  std::future<size_t> size_thread = std::async( std::launch::async,
						[&]() {
						  std::this_thread::sleep_for( test_time / 2 ) ;
						  return queue.size() ;
						} ) ;
  
  std::future<int> push_thread = std::async( std::launch::async,  
					     [&]() { 
					       auto start_time = std::chrono::steady_clock::now();
					       auto stop_time = start_time + test_time ;
					       int number = 0 ;
					       while( std::chrono::steady_clock::now() < stop_time ) {
						 ++ number ;
						 queue.enqueue( number ) ;
					       }
					       return number ;
					     } ) ;
  
  std::future<int> pop_thread = std::async( std::launch::async,  
					    [&]() { 
					      int memory = 0 ;
					      while( queue.try_dequeue_for( memory, timeout ) ) { ; }
					      return memory ;
					    } ) ;
  

  auto last_pushed_entry = push_thread.get() ;
  auto last_popped_entry = pop_thread.get() ;
  auto read_size = size_thread.get() ;

  BOOST_TEST_MESSAGE("Last pushed value: " << last_pushed_entry );
  BOOST_TEST_MESSAGE("Last popped value: " << last_popped_entry );
  BOOST_TEST_MESSAGE("Temp size: " << read_size );


  BOOST_REQUIRE_EQUAL( queue.size(), 0 );
  BOOST_REQUIRE_EQUAL( last_pushed_entry, last_popped_entry );
  
  BOOST_CHECK_GT( read_size, 0 );
  
}

 
