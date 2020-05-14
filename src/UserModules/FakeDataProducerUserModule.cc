#include "app-framework/UserModules/FakeDataProducerUserModule.hh"

#include <unistd.h>

#include <TRACE/trace.h>
#define TRACE_NAME "FakeDataProducer"

appframework::FakeDataProducerUserModule::FakeDataProducerUserModule(
  std::string name,
  std::vector<std::shared_ptr<BufferI>> inputs,
  std::vector<std::shared_ptr<BufferI>> outputs)
  : UserModule(name, inputs,outputs)
  , bufferTimeout_(100)
  , thread_(std::bind(&FakeDataProducerUserModule::do_work, this))
{
  if (inputs.size()) {
    throw std::runtime_error(
      "Invalid Configuration for FakeDataProducerUserModule: Input buffer "
      "provided!");
  }
  if (outputs.size() > 1) {
    throw std::runtime_error("Invalid Configuration for "
                             "FakeDataProducerUserModule: More than one Output "
                             "provided!");
  }

  outputBuffer_.reset(
    dynamic_cast<BufferInput<std::vector<int>>*>(&*outputs_[0]));
}

std::future<std::string>
appframework::FakeDataProducerUserModule::execute_command(std::string cmd)
{
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

std::string
appframework::FakeDataProducerUserModule::do_configure()
{
  // TODO: Get configuration from ConfigurationManager!
  nIntsPerVector_ = 10;
  starting_int_ = -4;
  ending_int_ = 14;
  wait_between_sends_ms_ = 1000;

  return "Success";
}

std::string
appframework::FakeDataProducerUserModule::do_start()
{
  thread_.start_working_thread_();
  return "Success";
}

std::string
appframework::FakeDataProducerUserModule::do_stop()
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
appframework::FakeDataProducerUserModule::do_work()
{
  int current_int = starting_int_;
  size_t counter = 0;
  while (thread_.thread_running()) {
    TLOG(TLVL_DEBUG) << "Creating output vector";
    std::vector<int> output(nIntsPerVector_);

    TLOG(TLVL_DEBUG) << "Start of fill loop";
    for (auto ii = 0; ii < nIntsPerVector_; ++ii) {
      output[ii] = current_int;
      if (++current_int > ending_int_)
        current_int = starting_int_;
    }
    TLOG(TLVL_INFO) << "Produced vector " << counter << " with contents "
                    << output;

    TLOG(TLVL_DEBUG) << "Pushing vector into outputBuffer";
    outputBuffer_->push(std::move(output), bufferTimeout_);

    TLOG(TLVL_DEBUG) << "Start of sleep between sends";
    usleep(wait_between_sends_ms_ * 1000);
    TLOG(TLVL_DEBUG) << "End of do_work loop";
    counter++;
  }
}

DEFINE_DUNE_USER_MODULE(appframework::FakeDataProducerUserModule)
