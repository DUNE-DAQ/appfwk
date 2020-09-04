/**
 * @file daq_application.cxx Main Application for the DAQ Framework, loads
 * DAQModules based on json configuration file
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/CommandLineInterpreter.hpp"
#include "appfwk/DAQModuleManager.hpp"
#include "appfwk/CommandFacility.hpp"

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
  using namespace dunedaq::appfwk;

  CommandLineInterpreter args;
  try {
    args = CommandLineInterpreter::parse(argc, argv);
  } catch (ers::Issue& e) {
    // Die but do it gracefully gracefully
    std::cout << "Command-line parsing failed. Error:" << std::endl;
    std::cout << e.message() << std::endl;
    exit(-1);
  }

  DAQModuleManager manager;
  auto cmdfac = makeCommandFacility(args.commandFacilityPluginName);
  cmdfac->run(manager);

}
