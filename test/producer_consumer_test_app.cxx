/**
 * @file producer_consumer_test_app.cxx
 *
 * This application loads a producer, two consumers and a fan-out DAQModules.
 * The producer creates vectors of ints, and the consumers each check that the
 * integers are increasing within each vector. They also see if there was a jump
 * indicating that a vector was missed (i.e. passed to the other consumer).
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "FakeDataConsumerDAQModule.hpp"
#include "FakeDataProducerDAQModule.hpp"
#include "appfwk/DAQProcess.hpp"
#include "appfwk/FanOutDAQModule.hpp"
#include "appfwk/StdDeQueue.hpp"

#include <memory>
#include <vector>

namespace dunedaq::appfwk {
/**
 * @brief ModuleList for the producer_consumer_test_app
*/
class producer_consumer_test_app_constructor : public GraphConstructor 
{
  // Inherited via ModuleList
  void ConstructGraph(DAQModuleMap& user_module_map,
                      CommandOrderMap& command_order_map) override
  {

    std::map<std::string, QueueConfig> queue_configuration;
    queue_configuration["producerToFanOut"].kind = QueueConfig::queue_kind::kStdDeQueue;
    queue_configuration["producerToFanOut"].size = 10;
    queue_configuration["fanOutToConsumer1"].kind = QueueConfig::queue_kind::kStdDeQueue;
    queue_configuration["fanOutToConsumer1"].size = 5;
    queue_configuration["fanOutToConsumer2"].kind = QueueConfig::queue_kind::kStdDeQueue;
    queue_configuration["fanOutToConsumer2"].size = 5;
    QueueRegistry::get().configure(queue_configuration);

    auto producerConfig = R"({ "output" : "producerToFanOut" })"_json;
    auto fanOutConfig =
      R"({ "input" : "producerToFanOut", "outputs" : [ "fanOutToConsumer1", "fanOutToConsumer2" ], "fanout_mode" : "RoundRobin" })"_json;
    auto consumer1Config = R"({ "input" : "fanOutToConsumer1" })"_json;
    auto consumer2Config = R"({ "input" : "fanOutToConsumer2" })"_json;

    user_module_map["producer"].reset(new FakeDataProducerDAQModule("prod"));
    user_module_map["producer"]->configure(producerConfig);
    user_module_map["fanOut"].reset(
      new FanOutDAQModule<std::vector<int>>("fanOut"));
    user_module_map["fanOut"]->configure(fanOutConfig);
    user_module_map["consumer1"].reset(new FakeDataConsumerDAQModule("C1"));
    user_module_map["consumer1"]->configure(consumer1Config);
    user_module_map["consumer2"].reset(new FakeDataConsumerDAQModule("C2"));
    user_module_map["consumer2"]->configure(consumer2Config);

    command_order_map["start"] = {
      "consumer1", "consumer2", "fanOut", "producer"
    };
    command_order_map["stop"] = { "producer" };
  }
};
} // namespace dunedaq::appfwk

/**
 * @brief producer_consumer_test_app main entry point
 * @param argc Number of arguments
 * @param argv Arguments
 * @return Status code from DAQProcess::listen
 */
int
main(int argc, char* argv[])
{
  auto args =
    appframework::CommandLineInterpreter::ParseCommandLineArguments(argc, argv);

  appframework::DAQProcess theDAQProcess(args);

  appframework::producer_consumer_test_app_constructor gc;
  theDAQProcess.register_modules( gc );

  return theDAQProcess.listen();
}
