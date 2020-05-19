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
#define TRACE_NAME "FakeDataProducer"

appframework::FakeDataProducerDAQModule::FakeDataProducerDAQModule(
    std::shared_ptr<BufferInput<std::vector<int>>> outputBuffer)
    : outputBuffer_(outputBuffer), bufferTimeout_(100),
      thread_(std::bind(&FakeDataProducerDAQModule::do_work, this)) {}

std::future<std::string>
appframework::FakeDataProducerDAQModule::execute_command(std::string cmd) {
  if (cmd == "configure" || cmd == "Configure") {
    return std::async(std::launch::async, [&] { return do_configure(); });
  }
  if (cmd == "start" || cmd == "Start") {
    return std::async(std::launch::async, [&] { return do_start(); });
  }
  if (cmd == "stop" || cmd == "Stop") {
    return std::async(std::launch::async, [&] { return do_stop(); });
  }

  return std::async(std::launch::async,
                    [] { return std::string("Unrecognized Command"); });
}

std::string appframework::FakeDataProducerDAQModule::do_configure() {
  nIntsPerVector_ = 10;
  starting_int_ = -4;
  ending_int_ = 14;
  wait_between_sends_ms_ = 1000;

  return "Success";
}

std::string appframework::FakeDataProducerDAQModule::do_start() {
  thread_.start_working_thread_();
  return "Success";
}

std::string appframework::FakeDataProducerDAQModule::do_stop() {
  thread_.stop_working_thread_();
  return "Success";
}

TraceStreamer &operator<<(TraceStreamer &t, std::vector<int> ints) {
  t << "{";
  bool first = true;
  for (auto &i : ints) {
    if (!first)
      t << ", ";
    first = false;
    t << i;
  }
  return t << "}";
}

void appframework::FakeDataProducerDAQModule::do_work() {
  int current_int = starting_int_;
  size_t counter = 0;
  while (thread_.thread_running()) {
    TLOG(TLVL_DEBUG) << "Creating output vector";
    std::vector<int> output(nIntsPerVector_);

    TLOG(TLVL_DEBUG) << "Start of fill loop";
    for (auto ii = 0; ii < nIntsPerVector_; ++ii) {
      output[ii] = current_int;
      ++current_int;
      if (current_int > ending_int_)
        current_int = starting_int_;
    }
    TLOG(TLVL_INFO) << "Produced vector " << counter << " with contents "
                    << output << " and size " << output.size();

    TLOG(TLVL_DEBUG) << "Pushing vector into outputBuffer";
    auto starttime = std::chrono::steady_clock::now();
    outputBuffer_->push(std::move(output), bufferTimeout_);
    auto endtime = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<decltype(bufferTimeout_)>(
            endtime - starttime) > bufferTimeout_) {
      TLOG(TLVL_WARNING)
          << "Timeout attempting to push vector onto outputBuffer";
    }

    TLOG(TLVL_DEBUG) << "Start of sleep between sends";
    std::this_thread::sleep_for(
        std::chrono::milliseconds(wait_between_sends_ms_));
    TLOG(TLVL_DEBUG) << "End of do_work loop";
    counter++;
  }
}
