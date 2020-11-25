/**
 *
 * @file queue_IO_check.cxx
 *
 * A low-level test of queue classes which implement the Queue
 * interface. We have a user-settable number of threads writing
 * elements to a queue while a user-settable number of threads reads
 * from the queue
 *
 * Run "queue_IO_check --help" to see options
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/FollyQueue.hpp"
#include "appfwk/StdDeQueue.hpp"

#include "TRACE/trace.h"

#include <boost/program_options.hpp>
namespace bpo = boost::program_options;

#include <chrono>
#include <future>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace dunedaq {
ERS_DECLARE_ISSUE(appfwk,                 ///< Namespace
                  ParameterDomainIssue,  ///< Issue class name
                  "ParameterDomainIssue: \"" << ers_messg << "\"",
		  ((std::string)ers_messg))
} // namespace dunedaq

namespace {

/**
 * @brief Type of the queue
 */
std::string queue_type = "StdDeQueue";

auto timeout = std::chrono::milliseconds(100); ///< Queue's timeout

/**
 * @brief Queue instance for test
 */
std::unique_ptr<dunedaq::appfwk::Queue<int>> queue = nullptr;

int nelements = 1000000; ///< Number of elements to push to the Queue (total)
int n_adding_threads = 1;      ///< Number of threads which will call push
int n_removing_threads = 1;    ///< Number of threads which will call pop

int avg_milliseconds_between_pushes = 0; ///< Target average rate of pushes
int avg_milliseconds_between_pops = 0;   ///< Target average rate of pops

// The enable_ options, when set to true, contain code that executes
// for each push/pop, which will of course affect the overall
// execution time of the threads while also adding info about the
// behavior of the system

bool enable_per_pushpop_timing = true;
bool enable_max_size_checking = true;

std::atomic<int> queue_size = 0;     ///< Queue's current size
std::atomic<int> max_queue_size = 0; ///< Queue's maximum size

std::atomic<int> timeout_pushes = 0;    ///< Number of pushes which timed out
std::atomic<int> timeout_pops = 0;      ///< Number of pops which timed out
std::atomic<int> throw_pushes = 0;      ///< Number of pushes which threw an exception
std::atomic<int> throw_pops = 0;        ///< Number of pops which threw an exception

double initial_capacity_used = 0; ///< The initial portion of the Queue which was full.

/**
 * @brief A time-based seed for the random number generators
 */
auto relatively_random_seed =
  std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() %
  1000;
std::default_random_engine generator(relatively_random_seed); ///< Random number generator with time-based seed
std::unique_ptr<std::uniform_int_distribution<int>> push_distribution =
  nullptr; ///< Random number distribution to use for push waits
std::unique_ptr<std::uniform_int_distribution<int>> pop_distribution =
  nullptr; ///< Random number distribution to use for pop waits

/**
 * @brief Put elements onto the queue
 */
void
add_things(const volatile bool& spinlock)
{
  const int npushes = nelements / n_adding_threads;
  auto starttime_push = std::chrono::steady_clock::now(); // Won't ever use the initialization value
  auto size_snapshot = queue_size.load(); // Unlike queue_size, only this thread writes to size_snapshot

  while (spinlock) {};  // Main program thread will set this to false, then this thread starts pushing

  const auto starttime = std::chrono::steady_clock::now();
  const auto starttime_system = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

  for (int i = 0; i < npushes; ++i) {

    if (avg_milliseconds_between_pushes > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds((*push_distribution)(generator)));
    }

    while (!queue->can_push()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    while (true) {
      try {

	if (!enable_per_pushpop_timing) {
	    queue->push(std::move(i), timeout);
	  } else {
	  starttime_push = std::chrono::steady_clock::now();
	  queue->push(std::move(i), timeout);
	  if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - starttime_push) >
	      timeout) {
	    timeout_pushes++;
	  }
	}

	if (enable_max_size_checking) {
	    size_snapshot = queue_size.fetch_add(1) + 1; // fetch_add returns previous value

	    if (size_snapshot > max_queue_size) {
	      max_queue_size = size_snapshot;
	    }
	  }

	break;
      } catch (const dunedaq::appfwk::QueueTimeoutExpired& err) {
	throw_pushes++;
	std::ostringstream msg;
	msg << "Thread #" << std::this_thread::get_id() << ": exception thrown on push #" << i << ": " << err.what();
	TLOG(TLVL_WARNING) << msg.str();
      }
    }
  }

  std::ostringstream msg;
  msg << "Thread #" << std::this_thread::get_id() << ": started at " << starttime_system << " ms since epoch, tried pushing " << npushes << " elements; time taken was " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - starttime).count() << " ms" << "\n";
  TLOG(TLVL_INFO) << msg.str();
}

/**
 * @brief Pop elements off of the queue
 */
void
remove_things(const volatile bool& spinlock)   
{
  const int npops = n_removing_threads > 0 ? nelements / n_removing_threads : 0;
  auto starttime_pop = std::chrono::steady_clock::now(); // Won't ever use the initialization value
  int val = -999;

  while (spinlock) {};  // Main program thread will set this to false, then this thread starts popping

  const auto starttime = std::chrono::steady_clock::now();
  const auto starttime_system = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

  for (int i = 0; i < npops; ++i) {

    if (avg_milliseconds_between_pops > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds((*pop_distribution)(generator)));
    }

    while (!queue->can_pop()) {
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    while (true) {
        try {

	  if (!enable_per_pushpop_timing) {
	      queue->pop(val, timeout);
	    } else {
	    starttime_pop = std::chrono::steady_clock::now();
	    queue->pop(val, timeout);

	    if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - starttime_pop) >
		timeout) 
	      timeout_pops++;
	  }

	  if (enable_max_size_checking) {
	      queue_size--;
	    }
	  break;

	} catch (const dunedaq::appfwk::QueueTimeoutExpired& e) {
	  throw_pops++;
	  std::ostringstream msg;
	  msg << "Thread #" << std::this_thread::get_id() << ": exception thrown on pop #" << i << ": " << e.what();
	  TLOG(TLVL_WARNING) << msg.str();
	}
    }
  }

  std::ostringstream msg;
  msg << "Thread #" << std::this_thread::get_id() << ": started at " << starttime_system << " ms since epoch, tried popping " << npops << " elements; time taken was " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - starttime).count() << " ms" << "\n";
  TLOG(TLVL_INFO) << msg.str();
}

} // namespace

int
main(int argc, char* argv[])
{

  std::ostringstream descstr;
  descstr << argv[0] << " known arguments ";

  std::ostringstream nelements_desc;
  nelements_desc << "# of elements you want pushed and/or popped (default is " << nelements << ")";

  std::ostringstream push_threads_desc;
  push_threads_desc << "# of threads you want pushing elements onto the queue (default is " << n_adding_threads << ")";

  std::ostringstream pop_threads_desc;
  pop_threads_desc << "# of threads you want popping elements off the queue (default is " << n_removing_threads << ")";

  std::ostringstream push_pause_desc;
  push_pause_desc << "average time in milliseconds between a thread's pushes (default is "
                  << avg_milliseconds_between_pushes << ")";

  std::ostringstream pop_pause_desc;
  pop_pause_desc << "average time in milliseconds between a thread's pops (default is " << avg_milliseconds_between_pops
                 << ")";

  std::ostringstream capacity_used_desc;
  capacity_used_desc << "fraction of the queue's capacity filled at start (default is " << initial_capacity_used << ")";

  bpo::options_description desc(descstr.str());
  desc.add_options()("queue_type",
                     bpo::value<std::string>(),
                     "Type of queue instance you want to test (default is "
                     "StdDeQueue) (supported "
                     "types are: StdDeQueue, FollySPSCQueue, FollyMPMCQueue)")(
    "nelements", bpo::value<int>(), nelements_desc.str().c_str())(
    "push_threads", bpo::value<int>(), push_threads_desc.str().c_str())(
    "pop_threads", bpo::value<int>(), pop_threads_desc.str().c_str())(
    "pause_between_pushes", bpo::value<int>(), push_pause_desc.str().c_str())(
    "pause_between_pops", bpo::value<int>(), pop_pause_desc.str().c_str())(
    "capacity", bpo::value<int>()->default_value(1000000000), "queue capacity")(
    "initial_capacity_used", bpo::value<double>(), capacity_used_desc.str().c_str())("help,h", "produce help message");

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

  int capacity = vm["capacity"].as<int>();

  if (queue_type == "StdDeQueue") {
    queue.reset(new dunedaq::appfwk::StdDeQueue<int>("StdDeQueue", static_cast<size_t>(capacity)));
  } else if (queue_type == "FollySPSCQueue") {
    queue.reset(new dunedaq::appfwk::FollySPSCQueue<int>("FollySPSCQueue", static_cast<size_t>(capacity)));
  } else if (queue_type == "FollyMPMCQueue") {
    queue.reset(new dunedaq::appfwk::FollyMPMCQueue<int>("FollyMPMCQueue", static_cast<size_t>(capacity)));
  } else {
    TLOG(TLVL_ERROR) << "Unknown queue type \"" << queue_type << "\" requested for testing";
    return 1;
  }

  if (vm.count("nelements")) {
    nelements = vm["nelements"].as<int>();

    if (nelements <= 0) {
      throw dunedaq::appfwk::ParameterDomainIssue(ERS_HERE, "# of elements must be a positive integer");
    }
  }

  if (vm.count("push_threads")) {

    n_adding_threads = vm["push_threads"].as<int>();

    if (n_adding_threads < 0) {
      throw dunedaq::appfwk::ParameterDomainIssue(ERS_HERE, "# of pushing threads must be non-negative");
    }
    if (queue_type == "FollySPSCQueue" && n_adding_threads != 0 && n_adding_threads != 1) {
      throw dunedaq::appfwk::ParameterDomainIssue(ERS_HERE, "# of pushing threads must 0 or 1 for SPSC queue");
    }
    if (n_adding_threads > 0 && nelements % n_adding_threads != 0) {
      std::ostringstream msg;
      msg << "# of pushing threads must divide into the # of elements (" << nelements << ") without a remainder";
      throw dunedaq::appfwk::ParameterDomainIssue(ERS_HERE, msg.str());
    }
  }

  if (vm.count("pop_threads")) {
    n_removing_threads = vm["pop_threads"].as<int>();

    if (n_removing_threads < 0) {
      throw dunedaq::appfwk::ParameterDomainIssue(ERS_HERE, "# of popping threads must be non-negative");
    }
    if (queue_type == "FollySPSCQueue" && n_removing_threads != 0 && n_removing_threads != 1) {
      throw dunedaq::appfwk::ParameterDomainIssue(ERS_HERE, "# of popping threads must 0 or 1 for SPSC queue");
    }
    if (n_removing_threads > 0 && nelements % n_removing_threads != 0) {
      std::ostringstream msg;
      msg << "# of popping threads must divide into the # of elements (" << nelements << ") without a remainder";
      throw dunedaq::appfwk::ParameterDomainIssue(ERS_HERE, msg.str());
    }
  }

  if (vm.count("pause_between_pushes")) {
    avg_milliseconds_between_pushes = vm["pause_between_pushes"].as<int>();

    if (avg_milliseconds_between_pushes < 0) {
      throw dunedaq::appfwk::ParameterDomainIssue(ERS_HERE, "Average # of milliseconds between pushes must be non-negative");
    }
  }

  if (vm.count("pause_between_pops")) {
    avg_milliseconds_between_pops = vm["pause_between_pops"].as<int>();

    if (avg_milliseconds_between_pops < 0) {
      throw dunedaq::appfwk::ParameterDomainIssue(ERS_HERE, "Average # of milliseconds between pops must be non-negative");
    }
  }

  if (vm.count("initial_capacity_used")) {
    initial_capacity_used = vm["initial_capacity_used"].as<double>();

    if (initial_capacity_used < 0 || initial_capacity_used > 1) {
      throw dunedaq::appfwk::ParameterDomainIssue(ERS_HERE, "Initial fractional capacity of queue which is used must lie in the range [0, 1]");
    }
  }

  push_distribution.reset(new std::uniform_int_distribution<int>(0, 2 * avg_milliseconds_between_pushes));
  pop_distribution.reset(new std::uniform_int_distribution<int>(0, 2 * avg_milliseconds_between_pops));


  TLOG(TLVL_INFO) << n_adding_threads << " thread(s) pushing " << nelements
                  << " elements between them, each thread has an average time of " << avg_milliseconds_between_pushes
                  << " milliseconds between pushes";
  TLOG(TLVL_INFO) << n_removing_threads << " thread(s) popping " << nelements
                  << " elements between them, each thread has an average time of " << avg_milliseconds_between_pops
                  << " milliseconds between pops";
  TLOG(TLVL_INFO) << "Queue of type " << queue_type << " has capacity for " << capacity << " elements";
  
  int elements_to_begin_with = static_cast<int>(initial_capacity_used * capacity);

  if (initial_capacity_used > 0) {
    
    TLOG(TLVL_INFO) << "Before test officially begins, pushing " << elements_to_begin_with << " elements onto the queue";
    for (int i_e = 0; i_e < elements_to_begin_with; ++i_e) {
      queue->push(-1, timeout);
    }
    queue_size = elements_to_begin_with;
    max_queue_size = elements_to_begin_with;
    TLOG(TLVL_INFO) << "Finished pre-test filling of the queue";
  }

  if (n_adding_threads > 0 && elements_to_begin_with + nelements > capacity) {
    std::ostringstream msg;
    msg << "The number of elements the queue is initially filled with (" << elements_to_begin_with << 
      ") plus the number of elements which will be pushed onto it (" << nelements << ") exceeds the queue's capacity (" 
	<< capacity << ")";
    if (n_removing_threads > 0) {
      TLOG(TLVL_WARNING) << msg.str();
    } else {
          TLOG(TLVL_ERROR) << msg.str();
          return 2;
    }
  }

  if (n_removing_threads > 0 && nelements > elements_to_begin_with) {
    std::ostringstream msg;
    msg << "The number of elements the queue is initially filled with (" << elements_to_begin_with << ") minus the number of elements which will be popped off of it (" << nelements << ") is less than zero";

    if (n_adding_threads > 0) {
      TLOG(TLVL_WARNING) << msg.str();
    } else {
      TLOG(TLVL_ERROR) << msg.str();
      return 3;
    }
  }

  bool spinlock = true;
  
  std::vector<std::thread> adders;
  std::vector<std::thread> removers;

  for (int i = 0; i < n_adding_threads; ++i) {
    adders.emplace_back(add_things, std::cref(spinlock));
  }

  for (int i = 0; i < n_removing_threads; ++i) {
    removers.emplace_back(remove_things, std::cref(spinlock));
  }

  // 20 ms is the pause Ron used when he originally implemented the
  // spinlock strategy in his logging package

  std::this_thread::sleep_for(std::chrono::milliseconds(20));
  spinlock = false;

  const auto starttime = std::chrono::steady_clock::now();
  for (auto& adder : adders) {
    adder.join();
  }

  for (auto& remover : removers) {
    remover.join();
  }
  const auto total_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - starttime).count();

  TLOG(TLVL_INFO) << "\n\nFinal results: ";

  if (enable_max_size_checking) {
    TLOG(TLVL_INFO) << "Max queue size during running was " << max_queue_size;
  } else {
    TLOG(TLVL_INFO) << "Disabled check for max queue size during running";
  }

  if (n_adding_threads > 0) {
    TLOG(TLVL_INFO) << "There were " << throw_pushes << " exception throws on push calls";
    
    if (enable_per_pushpop_timing) {
      TLOG(TLVL_INFO) << "There were " << timeout_pushes << " pushes which took longer than the provided timeout of " << std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count() << " ms\n";
    } else {
      TLOG(TLVL_INFO) << "Disabled count of slow pushes\n";
    }    
  }

  if (n_removing_threads > 0) {
    TLOG(TLVL_INFO) << "There were " << throw_pops << " exception throws on pop calls";
    
    if (enable_per_pushpop_timing) {
      TLOG(TLVL_INFO) << "There were " << timeout_pops << " pops which took longer than the provided timeout of " << std::chrono::duration_cast<std::chrono::milliseconds>(timeout).count() << " ms\n";
    } else {
      TLOG(TLVL_INFO) << "Disabled count of slow pops\n";
    }    
  }

  TLOG(TLVL_INFO) << "Total time from start of thread launch to the last thread wrapping up was " << total_time << " ms";

  return 0;
} // NOLINT
