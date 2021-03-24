/**
 * @file daq_application.cxx Main Application for the DAQ Framework, loads
 * DAQModules based on json configuration file
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/Application.hpp"
#include "appfwk/CommandLineInterpreter.hpp"
#include "appfwk/Issues.hpp"
#include "cmdlib/CommandFacility.hpp"
#include "logging/Logging.hpp"

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
  TLOG() << "Signal received: " << signal;
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

  dunedaq::logging::Logging().setup();

  // Setup signals
  std::signal(SIGABRT, signal_handler);
  std::signal(SIGQUIT, signal_handler);

  using namespace dunedaq;

  appfwk::CommandLineInterpreter args;
  try {
    args = appfwk::CommandLineInterpreter::parse(argc, argv);
  } catch (ers::Issue& e) {
    // Die but do it gracefully gracefully.
    ers::error(appfwk::BadCliUsage(ERS_HERE, e.message()));
    exit(-1);
  }

  // Create the Application
  appfwk::Application app(
    args.app_name, args.partition_name, args.rest_api, args.command_facility_plugin_name, args.info_service_plugin_name);

  app.init();
  app.run(run_marker);

  return 0;
}
