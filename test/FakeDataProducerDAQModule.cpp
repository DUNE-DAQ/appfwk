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
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <TRACE/trace.h>
/**
 * @brief Name used by TRACE TLOG calls from this source file
 */
#define TRACE_NAME "FakeDataProducer" // NOLINT

namespace dunedaq {
namespace appfwk {

FakeDataProducerDAQModule::FakeDataProducerDAQModule(const std::string& name)
  : DAQModule(name)
  , thread_(std::bind(&FakeDataProducerDAQModule::do_work, this, std::placeholders::_1))
  , outputQueue_(nullptr)
  , queueTimeout_(100)
{

  register_command("conf", &FakeDataProducerDAQModule::do_configure);
  register_command("start", &FakeDataProducerDAQModule::do_start);
  register_command("stop", &FakeDataProducerDAQModule::do_stop);
}

void
FakeDataProducerDAQModule::init(const nlohmann::json& init_data)
{
  outputQueue_.reset(new DAQSink<std::vector<int>>(init_data["output"].get<std::string>()));
}

void
FakeDataProducerDAQModule::do_configure(const data_t& data)
{
  nIntsPerVector_ = data.value<int>("nIntsPerVector", 10);
  starting_int_ = data.value<int>("starting_int", -4);
  ending_int_ = data.value<int>("ending_int", 14);
  wait_between_sends_ms_ = data.value<int>("wait_between_sends_ms", 1000);
  queueTimeout_ = std::chrono::milliseconds(data.value<int>("queue_timeout_ms", 100));
}

void
FakeDataProducerDAQModule::do_start(const data_t& /*data*/)
{
  thread_.start_working_thread();
}

void
FakeDataProducerDAQModule::do_stop(const data_t& /*data*/)
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
FakeDataProducerDAQModule::do_work(std::atomic<bool>& running_flag)
{
  int current_int = starting_int_;
  size_t counter = 0;
  std::ostringstream oss;

  while (running_flag.load()) {
    TLOG(TLVL_TRACE) << get_name() << ": Creating output vector";
    std::vector<int> output(nIntsPerVector_);

    TLOG(TLVL_TRACE) << get_name() << ": Start of fill loop";
    for (size_t ii = 0; ii < nIntsPerVector_; ++ii) {
      output[ii] = current_int;
      ++current_int;
      if (current_int > ending_int_)
        current_int = starting_int_;
    }
    oss << "Produced vector " << counter << " with contents " << output << " and size " << output.size();
    ers::debug(ProducerProgressUpdate(ERS_HERE, get_name(), oss.str()));
    oss.str("");

    TLOG(TLVL_TRACE) << get_name() << ": Pushing vector into outputQueue";
    try {
      outputQueue_->push(std::move(output), queueTimeout_);
    } catch(const QueueTimeoutExpired& ex) {
      ers::warning(ex);
    }

    TLOG(TLVL_TRACE) << get_name() << ": Start of sleep between sends";
    std::this_thread::sleep_for(std::chrono::milliseconds(wait_between_sends_ms_));
    TLOG(TLVL_TRACE) << get_name() << ": End of do_work loop";
    counter++;
  }
}

} // namespace appfwk
} // namespace dunedaq

DEFINE_DUNE_DAQ_MODULE(dunedaq::appfwk::FakeDataProducerDAQModule)
