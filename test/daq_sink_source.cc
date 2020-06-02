/**
 * @file daq_sink_source.cc Test application which demonstrates the functionality of the QueueRegistry, DAQSink, and DAQSource
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "app-framework/CommandFacility.hh"
#include "app-framework/DAQSink.hh"
#include "app-framework/DAQSource.hh"
#include "app-framework/StdDeQueue.hh"
#include "ers/ers.h"

namespace appframework {
std::unique_ptr<CommandFacility> CommandFacility::handle_ =
  std::unique_ptr<CommandFacility>();
} // namespace appframework

using namespace appframework;

int
main(int argc, char const* argv[])
{

  std::map<std::string, QueueConfig> queuemap = {
    { "dummy", { QueueConfig::std_deque, 100 } }
  };

  QueueRegistry::get()->configure(queuemap);

  auto sink = new DAQSink<std::string>("dummy");
  TLOG(TLVL_DEBUG) << "Expecting queue mismatch error";
  try {
    auto source = new DAQSource<int>("dummy");
  } catch (appframework::DAQSourceConstrutionFailed& e) {
    ers::error(e);
    TLOG(TLVL_DEBUG) << "OK, got it. Carrying on";
  }

  auto source = new DAQSource<std::string>("dummy");

  TLOG(TLVL_DEBUG) << "sink can push: " << sink->can_push();
  TLOG(TLVL_DEBUG) << "source can pop: " << source->can_pop();

  sink->push(std::move("hello"));
  std::string x = source->pop();

  TLOG(TLVL_DEBUG) << "x = " << x;

  for (size_t i(0); i < 101; ++i) {
    sink->push("aaAaaa");
  }
  return 0;
}