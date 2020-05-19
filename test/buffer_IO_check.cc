/**
 *
 * @file A low-level test of buffer classes which inherit both from
 * BufferOutput and BufferInput where we have a user-settable number
 * of threads writing elements to a buffer while a user-settable
 * number of threads reads from the buffer
 *
 * Run "buffer_IO_check --help" to see options
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "app-framework/Buffers/DequeBuffer.hh"

#include "TRACE/trace.h"

#include <boost/program_options.hpp>
namespace bpo = boost::program_options;

#include <chrono>
#include <future>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <thread>

namespace {

// TODO John Freeman, May-8-2020 (jcfree@fnal.gov)

// Will replace use of DequeBuffer in the unique_ptr with a base
// class which supports both push and pop operations if and when one
// becomes available

std::string buffer_type = "DequeBuffer";
auto timeout = std::chrono::microseconds(100);

// The decltype means "Have the buffer's push/pop functions expect a duration of
// the same type as the timeout we defined"
std::unique_ptr<appframework::DequeBuffer<int, decltype(timeout)>> buffer =
    nullptr;

constexpr int nelements = 100;
int n_adding_threads = 1;
int n_removing_threads = 1;

int avg_milliseconds_between_pushes = 5;
int avg_milliseconds_between_pops = 5;

std::atomic<size_t> max_buffer_size = 0;

std::atomic<int> push_attempts = 0;
std::atomic<int> pop_attempts = 0;
std::atomic<int> successful_pushes = 0;
std::atomic<int> successful_pops = 0;
std::atomic<int> timeout_pushes = 0;
std::atomic<int> timeout_pops = 0;
std::atomic<int> throw_pushes = 0;
std::atomic<int> throw_pops = 0;

double initial_capacity_used = 0;

auto relatively_random_seed =
    std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch())
        .count() %
    1000000;
std::default_random_engine generator(relatively_random_seed);
std::unique_ptr<std::uniform_int_distribution<int>> push_distribution = nullptr;
std::unique_ptr<std::uniform_int_distribution<int>> pop_distribution = nullptr;

void add_things() {

  for (int i = 0; i < nelements / n_adding_threads; ++i) {

    std::this_thread::sleep_for(
        std::chrono::microseconds((*push_distribution)(generator)));

    while (buffer->full()) {
      std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    push_attempts++;

    try {
      std::ostringstream msg;
      msg << "Thread #" << std::this_thread::get_id()
          << ": about to push value " << i << " onto buffer of size "
          << buffer->size();
      TLOG(TLVL_DEBUG) << msg.str();

      auto starttime = std::chrono::steady_clock::now();
      buffer->push(i, timeout);
      if (std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - starttime) < timeout) {
        successful_pushes++;
      } else {
        timeout_pushes++;
      }
      msg.str(std::string());
      msg << "Thread #" << std::this_thread::get_id() << ": completed push";
      TLOG(TLVL_DEBUG) << msg.str();

    } catch (const std::runtime_error &err) {
      TLOG(TLVL_WARNING) << "Exception thrown during push attempt: "
                         << err.what();
      throw_pushes++;
    }

    int size = buffer->size();

    if (size > max_buffer_size) {
      max_buffer_size = size;
    }
  }
}

void remove_things() {

  for (int i = 0; i < nelements / n_removing_threads; ++i) {

    int size = buffer->size();

    if (size > max_buffer_size) {
      max_buffer_size = size;
    }

    std::this_thread::sleep_for(
        std::chrono::microseconds((*pop_distribution)(generator)));

    while (buffer->empty()) {
      std::this_thread::sleep_for(std::chrono::microseconds(100));
    }

    pop_attempts++;
    int val = -999;
    try {
      std::ostringstream msg;
      msg << "Thread #" << std::this_thread::get_id()
          << ": about to pop from buffer of size " << buffer->size();
      TLOG(TLVL_DEBUG) << msg.str();

      auto starttime = std::chrono::steady_clock::now();
      val = buffer->pop(timeout);

      msg.str(std::string());
      msg << "Thread #" << std::this_thread::get_id()
          << ": completed pop, value is " << val;
      TLOG(TLVL_DEBUG) << msg.str();

      if (std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - starttime) < timeout) {
        successful_pops++;
      } else {
        timeout_pops++;
      }
    } catch (const std::runtime_error &e) {
      TLOG(TLVL_WARNING) << "Exception thrown during pop attempt: " << e.what();
      throw_pops++;
    }
  }
}

} // namespace ""

int main(int argc, char *argv[]) {

  std::ostringstream descstr;
  descstr << argv[0] << " known arguments ";

  std::ostringstream push_threads_desc;
  push_threads_desc
      << "# of threads you want pushing elements onto the buffer (default is "
      << n_adding_threads << ")";

  std::ostringstream pop_threads_desc;
  pop_threads_desc
      << "# of threads you want popping elements off the buffer (default is "
      << n_removing_threads << ")";

  std::ostringstream push_pause_desc;
  push_pause_desc
      << "average time in milliseconds between a thread's pushes (default is "
      << avg_milliseconds_between_pushes << ")";

  std::ostringstream pop_pause_desc;
  pop_pause_desc
      << "average time in milliseconds between a thread's pops (default is "
      << avg_milliseconds_between_pops << ")";

  std::ostringstream capacity_used_desc;
  capacity_used_desc
      << "fraction of the buffer's capacity filled at start (default is "
      << initial_capacity_used << ")";

  bpo::options_description desc(descstr.str());
  desc.add_options()("buffer_type", bpo::value<std::string>(),
                     "Type of buffer instance you want to test (default is "
                     "DequeBuffer) (supported "
                     "types are: DequeBuffer)")(
      "push_threads", bpo::value<int>(), push_threads_desc.str().c_str())(
      "pop_threads", bpo::value<int>(), pop_threads_desc.str().c_str())(
      "pause_between_pushes", bpo::value<int>(), push_pause_desc.str().c_str())(
      "pause_between_pops", bpo::value<int>(), pop_pause_desc.str().c_str())(
      "initial_capacity_used", bpo::value<double>(),
      capacity_used_desc.str().c_str())("help,h", "produce help message");

  bpo::variables_map vm;
  bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
  bpo::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n"; // NOLINT (TRACE prints an unnecessary warning
                               // suggesting that a streamer be implemented for
                               // boost::program_options::options_description)
    return 0;
  }

  if (vm.count("buffer_type")) {
    buffer_type = vm["buffer_type"].as<std::string>();
  }

  if (buffer_type == "DequeBuffer") {
    buffer.reset(new appframework::DequeBuffer<int, decltype(timeout)>);
  } else {
    TLOG(TLVL_ERROR) << "Unknown buffer type \"" << buffer_type
                     << "\" requested for testing";
    return 1;
  }

  if (vm.count("push_threads")) {
    n_adding_threads = vm["push_threads"].as<int>();

    if (n_adding_threads <= 0) {
      throw std::domain_error(
          "# of pushing threads must be a positive integer");
    }
  }

  if (vm.count("pop_threads")) {
    n_removing_threads = vm["pop_threads"].as<int>();

    if (n_removing_threads <= 0) {
      throw std::domain_error(
          "# of popping threads must be a positive integer");
    }
  }

  if (vm.count("pause_between_pushes")) {
    avg_milliseconds_between_pushes = vm["pause_between_pushes"].as<int>();

    if (avg_milliseconds_between_pushes < 0) {
      throw std::domain_error("Average # of milliseconds between pushes must "
                              "not be a negative number");
    }
  }

  if (vm.count("pause_between_pops")) {
    avg_milliseconds_between_pops = vm["pause_between_pops"].as<int>();

    if (avg_milliseconds_between_pops < 0) {
      throw std::domain_error("Average # of milliseconds between pops must not "
                              "be a negative number");
    }
  }

  if (vm.count("initial_capacity_used")) {
    initial_capacity_used = vm["initial_capacity_used"].as<double>();

    if (initial_capacity_used < 0 || initial_capacity_used > 1) {
      throw std::domain_error("Initial fractional capacity of buffer which is "
                              "used must lie in the range [0, 1]");
    }
  }

  push_distribution.reset(new std::uniform_int_distribution<int>(
      0, 2 * 1000 * avg_milliseconds_between_pushes));
  pop_distribution.reset(new std::uniform_int_distribution<int>(
      0, 2 * 1000 * avg_milliseconds_between_pops));

  TLOG(TLVL_INFO)
      << n_adding_threads << " thread(s) pushing " << nelements
      << " elements between them, each thread has an average time of "
      << avg_milliseconds_between_pushes << " milliseconds between pushes";
  TLOG(TLVL_INFO)
      << n_removing_threads << " thread(s) popping " << nelements
      << " elements between them, each thread has an average time of "
      << avg_milliseconds_between_pops << " milliseconds between pops";

  if (initial_capacity_used > 0) {

    int max_capacity = 1000000;

    if (buffer->capacity() <= max_capacity) {
      int elements_to_begin_with =
          static_cast<int>(initial_capacity_used * buffer->capacity());
      for (int i_e = 0; i_e < elements_to_begin_with; ++i_e) {
        buffer->push(-1, timeout);
      }
    } else {
      std::ostringstream msg;
      msg << "Since capacity of buffer exceeds " << max_capacity
          << ", the initial fractional used capacity of the buffer must be 0";
      throw std::domain_error(msg.str());
    }
  }

  std::vector<std::thread> adders;
  std::vector<std::thread> removers;

  for (int i = 0; i < n_adding_threads; ++i) {
    adders.emplace_back(add_things);
  }

  for (int i = 0; i < n_removing_threads; ++i) {
    removers.emplace_back(remove_things);
  }

  for (auto &adder : adders) {
    adder.join();
  }

  for (auto &remover : removers) {
    remover.join();
  }

  TLOG(TLVL_INFO) << "Max buffer size during running was " << max_buffer_size;
  TLOG(TLVL_INFO) << "Final buffer size at the end of running is "
                  << buffer->size();
  TLOG(TLVL_INFO) << push_attempts
                  << " push attempts made: " << successful_pushes
                  << " successful, " << timeout_pushes << " timeouts, "
                  << throw_pushes << " exception throws";
  TLOG(TLVL_INFO) << pop_attempts << " pop attempts made: " << successful_pops
                  << " successful, " << timeout_pops << " timeouts, "
                  << throw_pops << " exception throws";

  return 0;
} // NOLINT
