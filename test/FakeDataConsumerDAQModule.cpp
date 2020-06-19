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
/**
 * @brief Name used by TRACE TLOG calls from this source file
 */
#define TRACE_NAME "FakeDataConsumer" // NOLINT

#include <chrono>
#include <functional>
#include <thread>

namespace dunedaq::appfwk {

FakeDataConsumerDAQModule::FakeDataConsumerDAQModule(const std::string& name)
  : DAQModule(name)
  , queueTimeout_(100)
  , thread_(std::bind(&FakeDataConsumerDAQModule::do_work, this))
  , inputQueue_(nullptr)
{

  register_command("configure", &FakeDataConsumerDAQModule::do_configure);
  register_command("start",  &FakeDataConsumerDAQModule::do_start);
  register_command("stop",  &FakeDataConsumerDAQModule::do_stop);
}

void
FakeDataConsumerDAQModule::init() {
  inputQueue_.reset(new DAQSource<std::vector<int>>(get_config()["input"].get<std::string>()));
}


void
FakeDataConsumerDAQModule::do_configure(const std::vector<std::string>& args)
{

  nIntsPerVector_ = get_config().value<int>("nIntsPerVector", 10);
  starting_int_ = get_config().value<int>("starting_int", -4);
  ending_int_ = get_config().value<int>("ending_int", 14);

}

void
FakeDataConsumerDAQModule::do_start(const std::vector<std::string>& args)
{
  thread_.start_working_thread_();
}

void
FakeDataConsumerDAQModule::do_stop(const std::vector<std::string>& args)
{
  thread_.stop_working_thread_();
}

/**
 * @brief Format a std::vector<int> for TRACE
 * @param t TraceStreamer Instance
 * @param ints Vector to format
 * @return TraceStreamer Instance
 */
TraceStreamer&
operator<<(TraceStreamer& t, std::vector<int> ints)
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
FakeDataConsumerDAQModule::do_work()
{
  int current_int = starting_int_;
  int counter = 0;
  int fail_count = 0;
  std::vector<int> vec;

  while (thread_.thread_running()) {
    if (inputQueue_->can_pop()) {

      TLOG(TLVL_TRACE) << get_name() << ": Going to receive data from inputQueue";

      if (!inputQueue_->pop(vec, queueTimeout_)) {
        TLOG(TLVL_WARNING) << get_name() << ": Tried but failed to pop a value from an inputQueue";
        continue;
      }

      TLOG(TLVL_TRACE) << get_name() << ": Received vector of size " << vec.size();

      bool failed = false;

      TLOG(TLVL_TRACE) << get_name() << ": Starting processing loop";
      TLOG(TLVL_INFO) << get_name() << ": Received vector " << counter << ": " << vec;
      size_t ii = 0;
      for (auto& point : vec) {
        if (point != current_int) {
          if (ii != 0) {
            TLOG(TLVL_WARNING) << get_name() << ": Error in received vector " << counter << ", position " << ii
                               << ": Expected " << current_int << ", received " << point;
            failed = true;
          } else {
            TLOG(TLVL_INFO) << get_name() << ": Jump detected!";
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

  TLOG(TLVL_INFO) << get_name() << ": Processed " << counter << " vectors with " << fail_count << " failures.";
}

} // namespace dunedaq::appfwk

DEFINE_DUNE_DAQ_MODULE(dunedaq::appfwk::FakeDataConsumerDAQModule)
