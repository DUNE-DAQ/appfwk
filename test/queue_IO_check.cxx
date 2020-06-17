/**
 *
 * @file queue_IO_check.cxx
 *
 * A low-level test of queue classes where we have a user-settable number
 * of threads writing elements to a queue while a user-settable
 * number of threads reads from the queue
 *
 * Run "queue_IO_check --help" to see options
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/StdDeQueue.hpp"
#include "appfwk/FollyQueue.hpp"
#include "appfwk/FollyQueuePopT.hpp"

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

/**
 * @brief Type of the queue
 * @todo John Freeman, May-8-2020 (jcfree@fnal.gov)
 * Will replace use of StdDeQueue in the unique_ptr with a base
 * class which supports both push and pop operations if and when one
 * becomes available
 */
std::string queue_type = "StdDeQueue";

auto timeout = std::chrono::milliseconds(1); ///< Queue's timeout

  const bool pushes_only = false;
  const bool pops_only = false;

  static_assert( ! ( pushes_only && pops_only ));

constexpr int nelements = 10000000; ///< Number of elements to push to the Queue (total)

/**
 * @brief Queue instance for test
*/
  std::unique_ptr<dunedaq::appfwk::Queue<int>> queue = nullptr;

int n_adding_threads = 1; ///< Number of threads which will call push
int n_removing_threads = 1; ///< Number of threads which will call pop

int avg_milliseconds_between_pushes = 0; ///< Target average rate of pushes
int avg_milliseconds_between_pops = 0; ///< Target average rate of pops

std::atomic<size_t> queue_size = 0; ///< Queue's current size
std::atomic<size_t> max_queue_size = 0; ///< Queue's maximum size

std::atomic<int> push_attempts = 0; ///< Number of push attempts in the test
std::atomic<int> pop_attempts = 0; ///< Number of pop attempts in the test
std::atomic<int> successful_pushes = 0; ///< Number of successful pushes in the test
std::atomic<int> successful_pops = 0; ///< Number of successful pops in the test
std::atomic<int> timeout_pushes = 0; ///< Number of pushes which timed out
std::atomic<int> timeout_pops = 0; ///< Number of pops which timed out
std::atomic<int> throw_pushes = 0; ///< Number of pushes which threw an exception
std::atomic<int> throw_pops = 0; ///< Number of pops which threw an exception

double initial_capacity_used = 0; ///< The initial portion of the Queue which was full.

/**
 * @brief A time-based seed for the random number generators
*/
auto relatively_random_seed =
  std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch())
    .count() %
  1000;
std::default_random_engine generator(relatively_random_seed); ///< Random number generator with time-based seed
std::unique_ptr<std::uniform_int_distribution<int>> push_distribution = nullptr;///< Random number distribution to use for push waits
std::unique_ptr<std::uniform_int_distribution<int>> pop_distribution = nullptr; ///< Random number distribution to use for pop waits

/**
 * @brief Put elements onto the queue
*/
void
add_things()
{

  for (int i = 0; i < nelements / n_adding_threads; ++i) {

    if (avg_milliseconds_between_pushes > 0) {
      std::this_thread::sleep_for(
				  std::chrono::milliseconds((*push_distribution)(generator)));
    }

    while (!queue->can_push()) {
      TLOG(TLVL_INFO) << "queue->can_push() found to be false, performing external sleep of 100 ms before checking again";
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    push_attempts++;

    try {
      std::ostringstream msg;
      msg << "Thread #" << std::this_thread::get_id()
          << ": about to push value " << i << " onto queue with can_pop flag "
          << std::boolalpha << queue->can_pop();
      TLOG(TLVL_DEBUG) << msg.str();

      auto starttime = std::chrono::steady_clock::now();
      queue->push(std::move(i), timeout);
      if (std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - starttime) < timeout) {
        successful_pushes++;
        auto size = queue_size.fetch_add(1) + 1; // fetch_add returns previous value
        if (size > max_queue_size) {
          max_queue_size = size;
        }
      } else {
        timeout_pushes++;
      }
      msg.str(std::string());
      msg << "Thread #" << std::this_thread::get_id() << ": completed push";
      TLOG(TLVL_DEBUG) << msg.str();

    } catch (const std::runtime_error& err) {
      TLOG(TLVL_WARNING) << "Exception thrown during push attempt: "
                         << err.what();
      throw_pushes++;
    }
  }
}

/**
 * @brief Pop elements off of the queue
*/
void
remove_things()
{

  for (int i = 0; i < nelements / n_removing_threads; ++i) {

    if (avg_milliseconds_between_pops > 0) {
      std::this_thread::sleep_for(
				  std::chrono::milliseconds((*pop_distribution)(generator)));
    }

    while (!queue->can_pop()) {
      TLOG(TLVL_INFO) << "queue->can_pop() found to be false, performing external sleep of 100 ms before checking again";
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    pop_attempts++;
    int val = -999;
    try {
      std::ostringstream msg;
      msg << "Thread #" << std::this_thread::get_id()
          << ": about to pop from queue with can_push flag " << std::boolalpha
          << queue->can_push();
      TLOG(TLVL_DEBUG) << msg.str();

      auto starttime = std::chrono::steady_clock::now();

      if (queue_type == "FollySPSCQueuePopT") {
	dynamic_cast<dunedaq::appfwk::FollySPSCQueuePopT<int>*>(queue.get())->pop(val, timeout);
      } else if (queue_type == "FollyMPMCQueuePopT") {
	dynamic_cast<dunedaq::appfwk::FollyMPMCQueuePopT<int>*>(queue.get())->pop(val, timeout);
      } else if (queue_type == "FollySPSCQueue") {
	val = dynamic_cast<dunedaq::appfwk::FollySPSCQueue<int>*>(queue.get())->pop(timeout);
      } else if (queue_type == "FollyMPMCQueue") {
	val = dynamic_cast<dunedaq::appfwk::FollyMPMCQueue<int>*>(queue.get())->pop(timeout);
      } else if (queue_type == "StdDeQueue") {
	val = dynamic_cast<dunedaq::appfwk::StdDeQueue<int>*>(queue.get())->pop(timeout);
      }

      msg.str(std::string());
      msg << "Thread #" << std::this_thread::get_id()
          << ": completed pop, value is " << val;
      TLOG(TLVL_DEBUG) << msg.str();

      if (std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - starttime) < timeout) {
        successful_pops++;
        queue_size--;
      } else {
        timeout_pops++;
      }
    } catch (const std::runtime_error& e) {
      TLOG(TLVL_WARNING) << "Exception thrown during pop attempt: " << e.what();
      throw_pops++;
    }
  }
}

} // namespace ""

int
main(int argc, char* argv[])
{

  std::ostringstream descstr;
  descstr << argv[0] << " known arguments ";

  std::ostringstream push_threads_desc;
  push_threads_desc
    << "# of threads you want pushing elements onto the queue (default is "
    << n_adding_threads << ")";

  std::ostringstream pop_threads_desc;
  pop_threads_desc
    << "# of threads you want popping elements off the queue (default is "
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
    << "fraction of the queue's capacity filled at start (default is "
    << initial_capacity_used << ")";

  std::ostringstream queue_used_desc;
  queue_used_desc 
    << "Type of queue instance you want to test (default is " << queue_type <<
    ") (supported types are: StdDeQueue, FollySPSCQueue, FollyMPMCQueue, FollySPSCQueuePopT, FollyMPMCQueuePopT)";

  bpo::options_description desc(descstr.str());
  desc.add_options()("queue_type",
                     bpo::value<std::string>(), queue_used_desc.str().c_str())(
    "push_threads", bpo::value<int>(), push_threads_desc.str().c_str())(
    "pop_threads", bpo::value<int>(), pop_threads_desc.str().c_str())(
    "pause_between_pushes", bpo::value<int>(), push_pause_desc.str().c_str())(
    "pause_between_pops", bpo::value<int>(), pop_pause_desc.str().c_str())(
    "capacity", bpo::value<int>()->default_value(nelements*2), "queue capacity")(
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

  if (vm.count("queue_type")) {
    queue_type = vm["queue_type"].as<std::string>();
  }

  size_t capacity=vm["capacity"].as<int>();

  if (vm.count("push_threads")) {
    n_adding_threads = vm["push_threads"].as<int>();

    if (n_adding_threads <= 0) {
      throw std::domain_error(
        "# of pushing threads must be a positive integer");
    }
    if ((queue_type=="FollySPSCQueue" || queue_type=="FollySPSCQueuePopT") && n_adding_threads != 1) {
      throw std::domain_error(
        "# of pushing threads must be 1 for SPSC queue");
    }
  }

  if (vm.count("pop_threads")) {
    n_removing_threads = vm["pop_threads"].as<int>();

    if (n_removing_threads <= 0) {
      throw std::domain_error(
        "# of popping threads must be a positive integer");
    }
    if ((queue_type=="FollySPSCQueue" || queue_type=="FollySPSCQueuePopT") && n_removing_threads != 1) {
      throw std::domain_error(
        "# of popping threads must be 1 for SPSC queue");
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
      throw std::domain_error("Initial fractional capacity of queue which is "
                              "used must lie in the range [0, 1]");
    }
  }

  push_distribution.reset(new std::uniform_int_distribution<int>(
    0, 2 * avg_milliseconds_between_pushes));
  pop_distribution.reset(new std::uniform_int_distribution<int>(
    0, 2 * avg_milliseconds_between_pops));

  if (! pops_only ) {
  TLOG(TLVL_INFO)
    << n_adding_threads << " thread(s) pushing " << nelements
    << " elements between them, each thread has an average time of "
    << avg_milliseconds_between_pushes << " milliseconds between pushes";
  }

  if ( ! pushes_only ) {
  TLOG(TLVL_INFO)
    << n_removing_threads << " thread(s) popping " << nelements
    << " elements between them, each thread has an average time of "
    << avg_milliseconds_between_pops << " milliseconds between pops";
  }


  if (queue_type == "StdDeQueue") {
    queue.reset(new dunedaq::appfwk::StdDeQueue<int>("StdDeQueue"));
    dynamic_cast<dunedaq::appfwk::StdDeQueue<int>*>(queue.get())->SetSize(capacity);
  }
    else if (queue_type == "FollySPSCQueue") {
    queue.reset(new dunedaq::appfwk::FollySPSCQueue<int>("FollySPSCQueue"));
    dynamic_cast<dunedaq::appfwk::FollySPSCQueue<int>*>(queue.get())->SetSize(capacity);
  }
    else if (queue_type == "FollyMPMCQueue") {
    queue.reset(new dunedaq::appfwk::FollyMPMCQueue<int>("FollyMPMCQueue"));
    dynamic_cast<dunedaq::appfwk::FollyMPMCQueue<int>*>(queue.get())->SetSize(capacity);
  }
    else if (queue_type == "FollySPSCQueuePopT") {
    queue.reset(new dunedaq::appfwk::FollySPSCQueuePopT<int>("FollySPSCQueuePopT"));
    dynamic_cast<dunedaq::appfwk::FollySPSCQueuePopT<int>*>(queue.get())->SetSize(capacity);
  }
    else if (queue_type == "FollyMPMCQueuePopT") {
    queue.reset(new dunedaq::appfwk::FollyMPMCQueuePopT<int>("FollyMPMCQueuePopT"));
    dynamic_cast<dunedaq::appfwk::FollyMPMCQueuePopT<int>*>(queue.get())->SetSize(capacity);
  } else {
    TLOG(TLVL_ERROR) << "Unknown queue type \"" << queue_type
                     << "\" requested for testing";
    return 1;
  }

  int elements_to_begin_with = static_cast<int>(initial_capacity_used * capacity);
  if (elements_to_begin_with > 0) {
    TLOG(TLVL_INFO) << elements_to_begin_with << " were requested to fill the queue before starting; will fill...";
    for (int i_e = 0; i_e < elements_to_begin_with; ++i_e) {
      queue->push(-1, timeout);
    }
  }

  TLOG(TLVL_INFO) << "Queue starting out containing " << elements_to_begin_with << " elements";

  std::vector<std::thread> adders;
  std::vector<std::thread> removers;

  if (! pops_only ) {
    for (int i = 0; i < n_adding_threads; ++i) {
      adders.emplace_back(add_things);
    }
  }

  if (! pushes_only ) {
    for (int i = 0; i < n_removing_threads; ++i) {
      removers.emplace_back(remove_things);
    }
  }

  if (! pops_only ) {
    for (auto& adder : adders) {
      adder.join();
    }
  }

  if (! pushes_only ) {
    for (auto& remover : removers) {
      remover.join();
    }
  }

  TLOG(TLVL_INFO) << "Max queue size during running was " << max_queue_size;
  TLOG(TLVL_INFO) << "Final queue size at the end of running is " << queue_size;
  TLOG(TLVL_INFO) << push_attempts
                  << " push attempts made: " << successful_pushes
                  << " successful, " << timeout_pushes << " timeouts, "
                  << throw_pushes << " exception throws";
  TLOG(TLVL_INFO) << pop_attempts << " pop attempts made: " << successful_pops
                  << " successful, " << timeout_pops << " timeouts, "
                  << throw_pops << " exception throws";

  return 0;
} // NOLINT
