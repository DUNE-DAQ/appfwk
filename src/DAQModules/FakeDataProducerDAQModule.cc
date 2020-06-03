/**
 * @file The FakeDataProducerDAQModule class implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "app-framework/DAQModules/FakeDataProducerDAQModule.hh"

#include <chrono>
#include <thread>

#include <TRACE/trace.h>
#define TRACE_NAME "FakeDataProducer" // NOLINT

appframework::FakeDataProducerDAQModule::FakeDataProducerDAQModule(
  std::string name)
  : DAQModuleI(name)
  , queueTimeout_(100)
  , thread_(std::bind(&FakeDataProducerDAQModule::do_work, this))
  , outputQueue_(nullptr)
{}

void
appframework::FakeDataProducerDAQModule::execute_command(
  const std::string& cmd,
  const std::vector<std::string>& /*args*/)
{
  if (cmd == "configure" || cmd == "Configure") {
    do_configure();
  } else if (cmd == "start" || cmd == "Start") {
    do_start();
  } else if (cmd == "stop" || cmd == "Stop") {
    do_stop();
  } else {
    throw UnknownCommand(ERS_HERE, cmd);
  }
}

std::string
appframework::FakeDataProducerDAQModule::do_configure()
{

  outputQueue_.reset(
    new DAQSink<std::vector<int>>(configuration_["output"].get<std::string>()));

  nIntsPerVector_ = configuration_.value<int>("nIntsPerVector", 10);
  starting_int_ = configuration_.value<int>("starting_int", -4);
  ending_int_ = configuration_.value<int>("ending_int", 14);
  wait_between_sends_ms_ =
    configuration_.value<int>("wait_between_sends_ms", 1000);

  return "Success";
}

std::string
appframework::FakeDataProducerDAQModule::do_start()
{
  thread_.start_working_thread_();
  return "Success";
}

std::string
appframework::FakeDataProducerDAQModule::do_stop()
{
  thread_.stop_working_thread_();
  return "Success";
}

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
appframework::FakeDataProducerDAQModule::do_work()
{
  int current_int = starting_int_;
  size_t counter = 0;
  while (thread_.thread_running()) {
    TLOG(TLVL_DEBUG) << get_name() << ": Creating output vector";
    std::vector<int> output(nIntsPerVector_);

    TLOG(TLVL_DEBUG) << get_name() << ": Start of fill loop";
    for (auto ii = 0; ii < nIntsPerVector_; ++ii) {
      output[ii] = current_int;
      ++current_int;
      if (current_int > ending_int_)
        current_int = starting_int_;
    }
    TLOG(TLVL_INFO) << get_name() << ": Produced vector " << counter
                    << " with contents " << output << " and size "
                    << output.size();

    TLOG(TLVL_DEBUG) << get_name() << ": Pushing vector into outputQueue";
    auto starttime = std::chrono::steady_clock::now();
    outputQueue_->push(std::move(output), queueTimeout_);
    auto endtime = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<decltype(queueTimeout_)>(
          endtime - starttime) > queueTimeout_) {
      TLOG(TLVL_WARNING)
        << get_name() << ": Timeout attempting to push vector onto outputQueue";
    }

    TLOG(TLVL_DEBUG) << get_name() << ": Start of sleep between sends";
    std::this_thread::sleep_for(
      std::chrono::milliseconds(wait_between_sends_ms_));
    TLOG(TLVL_DEBUG) << get_name() << ": End of do_work loop";
    counter++;
  }
}

DEFINE_DUNE_DAQ_MODULE(appframework::FakeDataProducerDAQModule)
