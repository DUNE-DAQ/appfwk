/**
 * @file dynamic_loading_test_app to demonstrate loading Queues and DAQModules
 * from a configuration file
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include <fstream>
#include <memory>
#include <vector>

#include <nlohmann/json.hpp>

#include "app-framework-base/DAQModules/DAQModuleI.hh"
#include "app-framework-base/Queues/QueueI.hh"
#include "app-framework/DAQProcess.hh"

// for convenience
using json = nlohmann::json;

namespace appframework {

class dynamic_loading_test_app_ModuleList : public ModuleList
{
public:
  dynamic_loading_test_app_ModuleList(json config_json)
    : config_(config_json)
  {}

  // Inherited via ModuleList
  virtual void ConstructGraph(QueueMap& queue_map,
                              DAQModuleMap& user_module_map,
                              CommandOrderMap& command_order_map) override
  {
    for (auto& queue : config_["queues"].items()) {
      queue_map[queue.key()] = makeQueue(queue.value());
    }

    for (auto& module : config_["modules"].items()) {
      std::vector<std::shared_ptr<QueueI>> inputs;
      for (auto& input : module.value()["inputs"]) {
        inputs.push_back(queue_map[input]);
      }

      std::vector<std::shared_ptr<QueueI>> outputs;
      for (auto& output : module.value()["outputs"]) {
        outputs.push_back(queue_map[output]);
      }
      user_module_map[module.key()] = makeModule(
        module.value()["user_module_type"], module.key(), inputs, outputs);
    }

    for (auto& command : config_["commands"].items()) {
      std::list<std::string> command_order;
      for (auto& comm : command.value()) {
        command_order.push_back(comm);
      }
      command_order_map[command.key()] = command_order;
    }
  }

private:
  json config_;
};
} // namespace appframework

int
main(int argc, char* argv[])
{

  auto args =
    appframework::CommandLineInterpreter::ParseCommandLineArguments(argc, argv);

  appframework::DAQProcess theDAQProcess(args);

  json json_config;
  if (args.applicaitonConfigurationFile != "") {

    std::ifstream ifile(args.applicaitonConfigurationFile);
    ifile >> json_config;
  } else {
    json_config = R"(
        {
            "queues": {
                "producerToFanOut": "VectorIntStdDeQueue",
                "fanOutToConsumer1": "VectorIntStdDeQueue",
                "fanOutToConsumer2": "VectorIntStdDeQueue"
            },
            "modules": {
                "producer": {
                    "user_module_type": "FakeDataProducerDAQModule",
                    "inputs": [],
                    "outputs": ["producerToFanOut"]
                },
                "fanOut": {
                    "user_module_type": "VectorIntFanOutDAQModule",
                    "inputs": ["producerToFanOut"],
                    "outputs": ["fanOutToConsumer1", "fanOutToConsumer2" ]
                },
                "consumer1": {
                    "user_module_type": "FakeDataConsumerDAQModule",
                    "inputs": ["fanOutToConsumer1"],
                    "outputs": []
                },
                "consumer2": {
                    "user_module_type": "FakeDataConsumerDAQModule",
                    "inputs": ["fanOutToConsumer2"],
                    "outputs": []
                }
            },
            "commands": {
                "start": [ "consumer1", "consumer2", "fanOut", "producer" ],
                "stop": [ "producer" ]
            }
        }
    )"_json;
  }

  appframework::dynamic_loading_test_app_ModuleList ml(json_config);
  theDAQProcess.register_modules(ml);

  return theDAQProcess.listen();
}