/**
 * @file dynamic_loading_test_app to demonstrate loading Buffers and UserModules
 * from a configuration file
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include <memory>
#include <fstream>
#include <vector>

#include <nlohmann/json.hpp>

#include "app-framework-base/UserModules/UserModule.hh"
#include "app-framework-base/Buffers/BufferI.hh"
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
  virtual void ConstructGraph(BufferMap& buffer_map,
                              UserModuleMap& user_module_map,
                              CommandOrderMap& command_order_map) override
  {
    for (auto& buffer : config_["buffers"].items()) {
      buffer_map[buffer.key()] = makeBuffer(buffer.value());
    }

    for (auto& module : config_["modules"].items()) {
      std::vector<std::shared_ptr<BufferI>> inputs;
      for (auto& input : module.value()["inputs"]) {
        inputs.push_back(buffer_map[input]);
      }

      std::vector<std::shared_ptr<BufferI>> outputs;
      for (auto& output : module.value()["outputs"]) {
        outputs.push_back(buffer_map[output]);
      }
      user_module_map[module.key()] =
        makeModule(module.value()["user_module_type"],module.key(), inputs, outputs);
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
            "buffers": {
                "producerToFanOut": "VectorIntDequeBuffer",
                "fanOutToConsumer1": "VectorIntDequeBuffer",
                "fanOutToConsumer2": "VectorIntDequeBuffer"
            },
            "modules": {
                "producer": {
                    "user_module_type": "FakeDataProducerUserModule",
                    "inputs": [],
                    "outputs": ["producerToFanOut"]
                },
                "fanOut": {
                    "user_module_type": "VectorIntFanOutUserModule",
                    "inputs": ["producerToFanOut"],
                    "outputs": ["fanOutToConsumer1", "fanOutToConsumer2" ]
                },
                "consumer1": {
                    "user_module_type": "FakeDataConsumerUserModule",
                    "inputs": ["fanOutToConsumer1"],
                    "outputs": []
                },
                "consumer2": {
                    "user_module_type": "FakeDataConsumerUserModule",
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