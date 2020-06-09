/**
 * @file daq_application.cxx Main Application for the DAQ Framework, loads
 * DAQModules based on json configuration file
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/DAQModule.hpp"
#include "appfwk/DAQProcess.hpp"
#include "appfwk/Queue.hpp"
#include "appfwk/QueueRegistry.hpp"

#include <nlohmann/json.hpp>

#include <fstream>
#include <memory>
#include <vector>

/**
 * @brief Using namespace for convenience
 */
using json = nlohmann::json;

namespace dunedaq::appfwk {
/**
 * @brief ModuleList for daq_application
 */
class daq_application_constructor : public GraphConstructor 
{
public:
  /**
   * @brief Constructor for the daq_application_ModuleList
   * @param config_json Configuration file to be used to create the DAQModule graph
   */
  explicit daq_application_constructor( const json & config_json)
    : config_(config_json)
  {}

  // Inherited via ModuleList
  void ConstructGraph(DAQModuleMap& user_module_map,
		      CommandOrderMap& command_order_map) override
  {
    std::map<std::string, QueueConfig> queue_configuration;
    for (auto& queue : config_["queues"].items()) {
      QueueConfig qc;
      qc.kind = qc.stoqk(queue.value()["kind"].get<std::string>());
      qc.size = queue.value()["size"].get<size_t>();
      queue_configuration[queue.key()] = qc;
    }
    QueueRegistry::get().configure(queue_configuration);

    for (auto& module : config_["modules"].items()) {

      user_module_map[module.key()] =
        makeModule(module.value()["user_module_type"], module.key());
      user_module_map[module.key()]->configure(module.value());
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
} // namespace dunedaq::appfwk

/**
 * @brief Entry point for daq_application
 * @param argc Number of arguments
 * @param argv Arguments
 * @return Status Code
*/
int
main(int argc, char* argv[])
{

  auto args =
    dunedaq::appfwk::CommandLineInterpreter::ParseCommandLineArguments(argc, argv);

  dunedaq::appfwk::DAQProcess theDAQProcess(args);

  json json_config;
  if (args.applicaitonConfigurationFile != "") {

    std::ifstream ifile(args.applicaitonConfigurationFile);
    ifile >> json_config;
  } else {
    json_config = R"(
        {
            "queues": {
                "producerToFanOut": {"size": 10, "kind": "StdDeQueue"},
                "fanOutToConsumer1": {"size": 5, "kind": "StdDeQueue"},
                "fanOutToConsumer2":{"size": 5, "kind": "StdDeQueue"}
            },
            "modules": {
                "producer": {
                    "user_module_type": "FakeDataProducerDAQModule",
                    "output": "producerToFanOut"
                },
                "fanOut": {
                    "user_module_type": "VectorIntFanOutDAQModule",
                    "input": "producerToFanOut",
                    "outputs": ["fanOutToConsumer1", "fanOutToConsumer2" ],
                    "fanout_mode": "RoundRobin"
                },
                "consumer1": {
                    "user_module_type": "FakeDataConsumerDAQModule",
                    "input": "fanOutToConsumer1"
                },
                "consumer2": {
                    "user_module_type": "FakeDataConsumerDAQModule",
                    "input": "fanOutToConsumer2"
                }
            },
            "commands": {
                "start": [ "consumer1", "consumer2", "fanOut", "producer" ],
                "stop": [ "producer" ]
            }
        }
    )"_json;
  }

  dunedaq::appfwk::daq_application_constructor gc(json_config);
  theDAQProcess.register_modules( gc );

  return theDAQProcess.listen();
}
