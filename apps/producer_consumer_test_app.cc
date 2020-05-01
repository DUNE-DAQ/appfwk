/**
 * @file simple_test_app to show basic functionality of DAQProcess
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include <memory>
#include <vector>

#include "app-framework/Buffers/DequeBuffer.hh"
#include "app-framework/DAQProcess.hh"
#include "app-framework/UserModules/FakeDataConsumerUserModule.hh"
#include "app-framework/UserModules/FakeDataProducerUserModule.hh"
#include "app-framework/UserModules/FanOutUserModule.hh"

namespace appframework {

class producer_consumer_test_app_ModuleList : public ModuleList
{
  // Inherited via ModuleList
  virtual void ConstructGraph(BufferMap& buffer_map,
                              UserModuleMap& user_module_map,
                              CommandOrderMap& command_order_map) override
  {
    std::shared_ptr<DequeBuffer<std::vector<int>>> producerToFanOut(
      new DequeBuffer<std::vector<int>>());
    std::shared_ptr<DequeBuffer<std::vector<int>>> fanOutToConsumer1(
      new DequeBuffer<std::vector<int>>());
    std::shared_ptr<DequeBuffer<std::vector<int>>> fanOutToConsumer2(
      new DequeBuffer<std::vector<int>>());
    buffer_map["producerToFanOut"] = producerToFanOut;
    buffer_map["fanOutToConsumer1"] = fanOutToConsumer1;
    buffer_map["fanOutToConsumer2"] = fanOutToConsumer2;

    user_module_map["producer"].reset(
      new FakeDataProducerUserModule("prod", {}, { producerToFanOut }));
    user_module_map["fanOut"].reset(new FanOutUserModule<std::vector<int>>(
      "fanOut",
      { producerToFanOut },
      { fanOutToConsumer1, fanOutToConsumer2 }));
    user_module_map["consumer1"].reset(
      new FakeDataConsumerUserModule("C1", { fanOutToConsumer1 }, {}));
    user_module_map["consumer2"].reset(
      new FakeDataConsumerUserModule("C2", { fanOutToConsumer2 }, {}));

    command_order_map["start"] = {
      "consumer1", "consumer2", "fanOut", "producer"
    };
    command_order_map["stop"] = { "producer" };
  }
};
} // namespace appframework

int
main(int argc, char* argv[])
{

  auto args =
    appframework::CommandLineInterpreter::ParseCommandLineArguments(argc, argv);

  appframework::DAQProcess theDAQProcess(args);

  appframework::producer_consumer_test_app_ModuleList ml;
  theDAQProcess.register_modules(ml);

  return theDAQProcess.listen();
}