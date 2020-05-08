/**
 *
 * @file A low-level test of buffer classes which inherit both from
 * BufferOutput and BufferInput where we have a user-settable number
 * of threads writing elements to a buffer while a user-settable
 * number of threads reads from the buffer
 *
 * Run "buffer_concurrency_check --help" to see options
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

std::unique_ptr<appframework::DequeBuffer<int>> buffer = nullptr;

constexpr int nelements = 100;
int n_adding_threads = 2;
int n_removing_threads = 2;

int avg_milliseconds_between_pushes = 5;
int avg_milliseconds_between_pops = 5;

std::atomic<size_t> max_buffer_size = 0;

std::default_random_engine generator;
std::unique_ptr<std::uniform_int_distribution<int>> push_distribution = nullptr;
std::unique_ptr<std::uniform_int_distribution<int>> pop_distribution = nullptr;

void add_things() {

  for (int i = 0; i < nelements / n_adding_threads; ++i) {

    std::this_thread::sleep_for(
        std::chrono::milliseconds((*push_distribution)(generator)));

    if (!buffer->full()) {
      try {
        buffer->push(int(i)); // NOLINT, we're in-place-constructing an rvalue
                              // here, not casting
      } catch (const std::runtime_error &err) {
        TLOG(TLVL_WARNING) << "Exception thrown during push attempt: "
                           << err.what();
      }

      int size = buffer->size();

      if (size > max_buffer_size) {
        max_buffer_size = size;
      }
    }
  }
}

void remove_things() {

  std::this_thread::sleep_for(
      std::chrono::milliseconds((*pop_distribution)(generator)));

  for (int i = 0; i < nelements / n_removing_threads; ++i) {
    int val = -999;
    try {
      val = buffer->pop();
    } catch (const std::runtime_error &e) {
      TLOG(TLVL_WARNING) << "Exception thrown during pop attempt: " << e.what();
    }

    int size = buffer->size();

    if (size > max_buffer_size) {
      max_buffer_size = size;
    }
  }
}

} // namespace

int main(int argc, char *argv[]) {

  if (argc == 1) {
    TLOG(TLVL_WARNING)
        << "Call this program with the --help option to see how to use it";
    return 1;
  }

  std::ostringstream descstr;
  descstr << argv[0] << " known arguments ";

  bpo::options_description desc(descstr.str());
  desc.add_options()("bufferType,b", bpo::value<std::string>(),
                     "Type of buffer instance you want to test (supported "
                     "types are: DequeBuffer)")(
      "push_threads", bpo::value<int>(),
      "# of threads you want pushing elements onto the buffer")(
      "pop_threads", bpo::value<int>(),
      "# of threads you want popping elements off the buffer")(
      "pause_between_pushes", bpo::value<int>(),
      "average time in milliseconds between a thread's pushes")(
      "pause_between_pops", bpo::value<int>(),
      "average time in milliseconds between a thread's pops")(
      "help,h", "produce help message");

  bpo::variables_map vm;
  bpo::store(bpo::parse_command_line(argc, argv, desc), vm);
  bpo::notify(vm);

  if (vm.count("help")) {
    std::cout << desc << "\n"; // NOLINT (TRACE prints an unnecessary warning
                               // suggesting that a streamer be implemented for
                               // boost::program_options::options_description)
    return 0;
  }

  const std::string buffer_type = vm["bufferType"].as<std::string>();
  if (buffer_type == "DequeBuffer") {
    buffer.reset(new appframework::DequeBuffer<int>);
  } else {
    TLOG(TLVL_ERROR) << "Unknown buffer type \"" << buffer_type
                     << "\" requested for testing";
    return 1;
  }

  if (vm.count("push_threads")) {
    n_adding_threads = vm["push_threads"].as<int>();
  }

  if (vm.count("pop_threads")) {
    n_removing_threads = vm["pop_threads"].as<int>();
  }

  if (vm.count("pause_between_pushes")) {
    avg_milliseconds_between_pushes = vm["pause_between_pushes"].as<int>();
  }

  if (vm.count("pause_between_pops")) {
    avg_milliseconds_between_pops = vm["pause_between_pops"].as<int>();
  }

  push_distribution.reset(new std::uniform_int_distribution<int>(
      0, 2 * avg_milliseconds_between_pushes));
  pop_distribution.reset(new std::uniform_int_distribution<int>(
      0, 2 * avg_milliseconds_between_pops));

  TLOG(TLVL_INFO)
      << n_adding_threads
      << " thread(s) pushing elements, each thread has an average time of "
      << avg_milliseconds_between_pushes << " milliseconds between pushes";
  TLOG(TLVL_INFO)
      << n_removing_threads
      << " thread(s) popping elements, each thread has an average time of "
      << avg_milliseconds_between_pops << " milliseconds between pops";

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

  return 0;
}
