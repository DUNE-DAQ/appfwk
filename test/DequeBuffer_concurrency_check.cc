/**
 *
 * @file A low-level DequeBuffer test in which we use multiple threads
 * to try reading elements from a DequeBuffer while writing elements
 * with another thread
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */


#include "app-framework/Buffers/DequeBuffer.hh"

#include "TRACE/trace.h"

#include <chrono>
#include <future>
#include <random>
#include <sstream>
#include <stdexcept>
#include <thread>

namespace {

  appframework::DequeBuffer<int> buffer;

  const int nelements = 100;
  const int n_removing_threads = 5;

  const int average_millisecond_wait = 5;

  std::atomic<size_t> max_buffer_size = 0;

  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(0,2*average_millisecond_wait);

  void add_things() {

    for (int i = 0; i < nelements; ++i) {
      std::this_thread::sleep_for(std::chrono::milliseconds(distribution(generator)));
      if (! buffer.full()) { 
	try {
	  buffer.push(int(i)); 
	} catch (const std::runtime_error& err) {
	  TLOG(TLVL_WARNING) << "Exception thrown during push attempt: " << err.what();
	}
      }
    }
  }

  void remove_things() {

    for(int i = 0; i < nelements/n_removing_threads; ++i) {
      int val = -999;
      try {
	val = buffer.pop();
      } catch (const std::runtime_error& e) {
	TLOG(TLVL_WARNING) << "Exception thrown during pop attempt: " << e.what();
      }

      int size = buffer.size();

      if (size > max_buffer_size) {
	max_buffer_size = size;
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(distribution(generator)));
    }
    }

} // namespace ""


int main() {

  TLOG(TLVL_INFO) << "Will try pushing " << nelements << " values to a DequeBuffer with one thread while popping them off with " << n_removing_threads << " threads";

  std::thread adder(add_things);

  std::vector<std::thread> removers;

  for (int i = 0; i < n_removing_threads; ++i) {
    removers.emplace_back(remove_things);
  }

  adder.join();

  for (auto& remover : removers) {
    remover.join();
  }

  TLOG(TLVL_INFO) << "Max buffer size during running was " << max_buffer_size;
  TLOG(TLVL_INFO) << "Final buffer size at the end of running is " << buffer.size();

  return 0;
}
