#include "app-framework-base/Services/CommandFacility.hh"
#include "app-framework/DAQSource.hh"
#include "app-framework/DAQSink.hh"
#include "app-framework/Queues/NamedStdDeQueue.hh"

namespace appframework {
std::unique_ptr<CommandFacility> CommandFacility::handle_ =
  std::unique_ptr<CommandFacility>();
}

using namespace appframework;

int main(int argc, char const *argv[])
{
    auto sink = new DAQSink<std::string>("dummy", QueueRegistry::std_deque, 100);
    auto source = new DAQSource<std::string>("dummy", QueueRegistry::std_deque, 100);


    std::cout << "sink can push: " << sink->can_push() << std::endl;
    std::cout << "source can pop: " << source->can_pop() << std::endl;

    sink->push("hello");
    std::string x = source->pop();

    std::cout << "x = " << x << std::endl;

    for ( size_t i(0); i<101; ++i) {
        sink->push("aaAaaa");
    }
    /* code */
    return 0;
}