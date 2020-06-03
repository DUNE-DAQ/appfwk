#include "app-framework/Services/CommandFacility.hh"
#include "app-framework/DAQSink.hh"
#include "app-framework/DAQSource.hh"
#include "app-framework/Queues/StdDeQueue.hh"
#include "ers/ers.h"

namespace appframework {
std::unique_ptr<CommandFacility> CommandFacility::handle_ =
  std::unique_ptr<CommandFacility>();
}

using namespace appframework;

int
main(int argc, char const* argv[])
{

  std::map<std::string, QueueConfig> queuemap = {
    { "dummy", { QueueConfig::std_deque, 100 } }
  };

  QueueRegistry::get()->configure(queuemap);

  auto sink = new DAQSink<std::string>("dummy");
  std::cout << "Expecting queue mismatch error" << std::endl;
  try {
    auto source = new DAQSource<int>("dummy");
  } catch (appframework::DAQSourceConstrutionFailed& e) {
    ers::error(e);
    std::cout << "OK, got it. Carrying on" << std::endl;
  }

  auto source = new DAQSource<std::string>("dummy");

  std::cout << "sink can push: " << sink->can_push() << std::endl;
  std::cout << "source can pop: " << source->can_pop() << std::endl;

  sink->push(std::move("hello"));
  std::string x = source->pop();

  std::cout << "x = " << x << std::endl;

  for (size_t i(0); i < 101; ++i) {
    sink->push("aaAaaa");
  }
  /* code */
  return 0;
}