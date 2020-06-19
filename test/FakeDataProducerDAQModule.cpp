/**
 * @file FakeDataProducerDAQModule.cc FakeDataProducerDAQModule class
 * implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "FakeDataProducerDAQModule.hpp"

#include <chrono>
#include <thread>
#include <utility>

#include <TRACE/trace.h>
/**
 * @brief Name used by TRACE TLOG calls from this source file
 */
#define TRACE_NAME "FakeDataProducer" // NOLINT

namespace dunedaq {
namespace appfwk {

FakeDataProducerDAQModule::FakeDataProducerDAQModule(const std::string& name)
  : DAQModule(name)
  , thread_(std::bind(&FakeDataProducerDAQModule::do_work, this))
  , outputQueue_(nullptr)
  , queueTimeout_(100)
{

  register_command("configure", &FakeDataProducerDAQModule::do_configure);
  register_command("start",  &FakeDataProducerDAQModule::do_start);
  register_command("stop",  &FakeDataProducerDAQModule::do_stop);
}

void FakeDataProducerDAQModule::init() {
  outputQueue_.reset(new DAQSink<std::vector<int>>(get_config()["output"].get<std::string>()));
}

void
FakeDataProducerDAQModule::do_configure([[maybe_unused]] const std::vector<std::string>& args)
{
  nIntsPerVector_ = get_config().value<int>("nIntsPerVector", 10);
  starting_int_ = get_config().value<int>("starting_int", -4);
  ending_int_ = get_config().value<int>("ending_int", 14);
  wait_between_sends_ms_ = get_config().value<int>("wait_between_sends_ms", 1000);
}

void
FakeDataProducerDAQModule::do_start([[maybe_unused]] const std::vector<std::string>& args)
{
  thread_.start_working_thread_();
}

void
FakeDataProducerDAQModule::do_stop([[maybe_unused]] const std::vector<std::string>& args)
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
FakeDataProducerDAQModule::do_work()
{
  int current_int = starting_int_;
  size_t counter = 0;
  while (thread_.thread_running()) {
    TLOG(TLVL_TRACE) << get_name() << ": Creating output vector";
    std::vector<int> output(nIntsPerVector_);

    TLOG(TLVL_TRACE) << get_name() << ": Start of fill loop";
    for (size_t ii = 0; ii < nIntsPerVector_; ++ii) {
      output[ii] = current_int;
      ++current_int;
      if (current_int > ending_int_)
        current_int = starting_int_;
    }
    TLOG(TLVL_TRACE) << get_name() << ": Produced vector " << counter << " with contents " << output << " and size "
                    << output.size();

    TLOG(TLVL_TRACE) << get_name() << ": Pushing vector into outputQueue";
    outputQueue_->push(std::move(output), queueTimeout_);

    TLOG(TLVL_TRACE) << get_name() << ": Start of sleep between sends";
    std::this_thread::sleep_for(std::chrono::milliseconds(wait_between_sends_ms_));
    TLOG(TLVL_TRACE) << get_name() << ": End of do_work loop";
    counter++;
  }
}

} // namespace appfwk 
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::appfwk::FakeDataProducerDAQModule)
