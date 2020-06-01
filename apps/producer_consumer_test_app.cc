/**
 * @file simple_test_app to show basic functionality of DAQProcess
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "app-framework/DAQModules/FakeDataConsumerDAQModule.hh"
#include "app-framework/DAQModules/FakeDataProducerDAQModule.hh"
#include "app-framework/DAQModules/FanOutDAQModule.hh"
#include "app-framework/DAQProcess.hh"
#include "app-framework/Queues/StdDeQueue.hh"

#include <memory>
#include <vector>

namespace appframework {

class producer_consumer_test_app_ModuleList : public ModuleList
{
  // Inherited via ModuleList
  void ConstructGraph(DAQModuleMap& user_module_map,
                      CommandOrderMap& command_order_map) override
  {

    user_module_map["producer"].reset(
      new FakeDataProducerDAQModule("prod"));
    user_module_map["fanOut"].reset(new FanOutDAQModule<std::vector<int>>(
      "fanOut"));
    user_module_map["consumer1"].reset(
      new FakeDataConsumerDAQModule("C1"));
    user_module_map["consumer2"].reset(
      new FakeDataConsumerDAQModule("C2"));

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
