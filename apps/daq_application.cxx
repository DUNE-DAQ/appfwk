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

#include "ers/Issue.h"
#include <nlohmann/json.hpp>

#include <fstream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

/**
 * @brief Using namespace for convenience
 */
using json = nlohmann::json;

namespace dunedaq {

/**
  * @brief InvalidConfiguration ERS Issue
 */
ERS_DECLARE_ISSUE(appfwk,               // namespace
		  InvalidConfiguration, // issue class name
		  "Invalid configuration detected in daq_application: module with name "
                  << module_name << " could not be added to application graph!", // message
		  ((std::string)module_name)) // parameters

namespace appfwk {
/**
 * @brief ModuleList for daq_application
 */
class daq_application_constructor : public GraphConstructor
{
public:
  /**
   * @brief Constructor for the daq_application_ModuleList
   * @param config_json Configuration file to be used to create the DAQModule
   * graph
   */
  explicit daq_application_constructor(const json& config_json)
    : config_(config_json)
  {}

  // Inherited via ModuleList
  void ConstructGraph(DAQModuleMap& user_module_map, CommandOrderMap& command_order_map) const override
  {
    std::map<std::string, QueueConfig> queue_configuration;
    for (auto& queue : config_["queues"].items()) {
      QueueConfig qc;
      qc.kind = qc.stoqk(queue.value()["kind"].get<std::string>());
      qc.capacity = queue.value()["capacity"].get<size_t>();
      queue_configuration[queue.key()] = qc;
    }

    QueueRegistry::get().configure(queue_configuration);

    for (auto& module : config_["modules"].items()) {
      auto [modit, done] =
        user_module_map.emplace(module.key(), makeModule(module.value()["user_module_type"], module.key()));
      if (!done) {
	throw dunedaq::appfwk::InvalidConfiguration(ERS_HERE, module.key());
      }
      modit->second->do_init(module.value());
    }

    for (auto& command : config_["commands"].items()) {
      std::list<std::string> command_order;
      for (const auto& comm : command.value()) {
        command_order.push_back(comm);
      }
      command_order_map[command.key()] = command_order;
    }
  }

private:
  const json config_;
};
} // namespace appfwk

/**
 * @brief NoConfiguration ERS Issue
 */
ERS_DECLARE_ISSUE(appfwk,          // namespace
                  NoConfiguration, // issue class name
                  "No configuration file given to daq_application; re-run with "
                  "daq_application -h to see options!", // message
)
} // namespace dunedaq

/**
 * @brief Entry point for daq_application
 * @param argc Number of arguments
 * @param argv Arguments
 * @return Status Code
 */
int
main(int argc, char* argv[])
{

  auto args = dunedaq::appfwk::CommandLineInterpreter::ParseCommandLineArguments(argc, argv);

  dunedaq::appfwk::DAQProcess theDAQProcess(args);

  json json_config;
  if (args.applicationConfigurationFile != "") {

    std::ifstream ifile(args.applicationConfigurationFile);
    ifile >> json_config;
  } else {
    throw dunedaq::appfwk::NoConfiguration(ERS_HERE);
  }

  dunedaq::appfwk::daq_application_constructor gc(json_config);
  theDAQProcess.register_modules(gc);

  return theDAQProcess.listen();
}
