/**
 * @file FakeDataConsumerDAQModule.cxx FakeDataConsumerDAQModule class
 * implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "FakeDataConsumerDAQModule.hpp"

#include "TRACE/trace.h"
#include <ers/ers.h>

/**
 * @brief Name used by TRACE TLOG calls from this source file
 */
#define TRACE_NAME "FakeDataConsumer" // NOLINT

#include <chrono>
#include <functional>
#include <string>
#include <thread>
#include <vector>

namespace dunedaq::appfwk {

FakeDataConsumerDAQModule::FakeDataConsumerDAQModule(const std::string& name)
  : DAQModule(name)
  , thread_(std::bind(&FakeDataConsumerDAQModule::do_work, this, std::placeholders::_1))
  , queueTimeout_(100)
  , inputQueue_(nullptr)
{

  register_command("configure", &FakeDataConsumerDAQModule::do_configure);
  register_command("start", &FakeDataConsumerDAQModule::do_start);
  register_command("stop", &FakeDataConsumerDAQModule::do_stop);
}

void
FakeDataConsumerDAQModule::init(const nlohmann::json& init_data)
{
  inputQueue_.reset(new DAQSource<std::vector<int>>(init_data["input"].get<std::string>()));
}

void
FakeDataConsumerDAQModule::do_configure(const data_t& data)
{

  nIntsPerVector_ = data.value<int>("nIntsPerVector", 10);
  starting_int_ = data.value<int>("starting_int", -4);
  ending_int_ = data.value<int>("ending_int", 14);
  queueTimeout_ = std::chrono::milliseconds(data.value<int>("queue_timeout_ms", 100));
}

void
FakeDataConsumerDAQModule::do_start(const data_t& /*data*/)
{
  thread_.start_working_thread();
}

void
FakeDataConsumerDAQModule::do_stop(const data_t& /*data*/)
{
  thread_.stop_working_thread();
}

/**
 * @brief Format a std::vector<int> to a stream
 * @param t ostream Instance
 * @param ints Vector to format
 * @return ostream Instance
 */
std::ostream&
operator<<(std::ostream& t, std::vector<int> ints)
{
  t << "{";
  bool first = true;
  for (auto& i : ints) {
    if (!first)
      t << ", ";
    first = false;
    t << i;
  }
  return t << "}";
}

void
FakeDataConsumerDAQModule::do_work(std::atomic<bool>& running_flag)
{
  int current_int = starting_int_;
  int counter = 0;
  int fail_count = 0;
  std::vector<int> vec;
  std::ostringstream oss;

  while (running_flag.load()) {
    if (inputQueue_->can_pop()) {

      TLOG(TLVL_TRACE) << get_name() << ": Going to receive data from inputQueue";

      try {
        inputQueue_->pop(vec, queueTimeout_);
      } catch (const dunedaq::appfwk::QueueTimeoutExpired& excpt) {
        continue;
      }

      TLOG(TLVL_TRACE) << get_name() << ": Received vector of size " << vec.size();

      bool failed = false;

      TLOG(TLVL_TRACE) << get_name() << ": Starting processing loop";
      oss << "Received vector " << counter << ": " << vec;
      ers::debug(ConsumerProgressUpdate(ERS_HERE, get_name(), oss.str()));
      oss.str("");

      size_t ii = 0;
      for (auto& point : vec) {
        if (point != current_int) {
          if (ii != 0) {
            ers::warning(ConsumerErrorDetected(ERS_HERE, get_name(), counter, ii, current_int, point));
            failed = true;
          } else {
            ers::info(
              ConsumerProgressUpdate(ERS_HERE, get_name(), std::string("Jump detected at ") + std::to_string(counter)));
          }
          current_int = point;
        }
        ++current_int;
        if (current_int > ending_int_)
          current_int = starting_int_;
        ++ii;
      }
      TLOG(TLVL_TRACE) << get_name() << ": Done with processing loop, failed=" << failed;
      if (failed)
        fail_count++;

      counter++;
    } else {
      std::this_thread::sleep_for(std::chrono::seconds(1));
    }
  }

  oss << ": Processed " << counter << " vectors with " << fail_count << " failures.";
  ers::info(ConsumerProgressUpdate(ERS_HERE, get_name(), oss.str()));
}

} // namespace dunedaq::appfwk

DEFINE_DUNE_DAQ_MODULE(dunedaq::appfwk::FakeDataConsumerDAQModule)
