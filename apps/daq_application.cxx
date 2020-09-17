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
  return 0;
}

