/**
 * @file daq_application.cxx Main Application for the DAQ Framework, loads
 * DAQModules based on json configuration file
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "Application.hpp"
#include "CommandLineInterpreter.hpp"
#include "appfwk/Issues.hpp"
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


  // Setup signals
  // std::signal(SIGABRT, signal_handler);
  // std::signal(SIGTERM, signal_handler);
  std::signal(SIGINT, signal_handler);
  std::signal(SIGQUIT, signal_handler);
  // std::signal(SIGHUP, signal_handler);

  using namespace dunedaq;

  appfwk::CommandLineInterpreter args;
  try {
    args = appfwk::CommandLineInterpreter::parse(argc, argv);
  } catch ( bpo::error const& e ) {
    // Die but do it gracefully gracefully.
    std::cerr << "Failed to interpret command line: " << e.what();
    exit(1);
  }

  if (args.help_requested) {
    exit(0);
  }

  // up to here it was not possible to use ERS messages
  
  dunedaq::logging::Logging().setup( args.session_name, args.app_name );

  // from now on, it's possible to use ERS messages
  
  // Create the Application
  appfwk::Application app(args.app_name, args.session_name,
			  args.command_facility_plugin_name,
			  args.conf_service_plugin_name);

  app.init();
  app.run(run_marker);

  TLOG() << "Application " << args.session_name << '.' << args.app_name << " exiting.";
  return 0;
}
