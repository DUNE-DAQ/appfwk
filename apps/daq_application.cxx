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
#include "cmdlib/CommandFacility.hpp"

#include "ers/Issue.h"
#include "nlohmann/json.hpp"

#include <csignal>
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
 * @brief Global atomic for process lifetime
 */
std::atomic<bool> run_marker{ true };

/**
 * @brief Signal handler for graceful stop
 */
static void
signal_handler(int signal)
{
  std::cout << "Signal received: " << signal << '\n'; // NOLINT(runtime/output_format)
  run_marker.store(false);
}

/**
 * @brief Entry point for daq_application
 * @param argc Number of arguments
 * @param argv Arguments
 * @return Status Code
 */
int
main(int argc, char* argv[])
{
  // Setup signals
  std::signal(SIGABRT, signal_handler);
  std::signal(SIGQUIT, signal_handler);

  using namespace dunedaq;

  appfwk::CommandLineInterpreter args;
  try {
    args = appfwk::CommandLineInterpreter::parse(argc, argv);
  } catch (ers::Issue& e) {
    // Die but do it gracefully gracefully.
    // Use of std::cout annoys the linter.
    std::cout << "Command-line parsing failed. Error:" << std::endl; // NOLINT(runtime/output_format)
    std::cout << e.message() << std::endl;                           // NOLINT(runtime/output_format)
    exit(-1);
  }

  // DAQModuleManager commandable
  appfwk::DAQModuleManager manager;

  // CommandFacility
  auto cmdfac = cmdlib::makeCommandFacility(args.m_command_facility_plugin_name);

  // Add commanded object to CF
  cmdfac->set_commanded(manager);

  // Run until global signal
  cmdfac->run(run_marker);

  return 0;
}
