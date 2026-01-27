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

#include "logging/Logging.hpp"

#include <csignal>
#include <fstream>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

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

int
main(int argc, char* argv[])
{

  // Setup signals
  std::signal(SIGINT, signal_handler);
  std::signal(SIGQUIT, signal_handler);

  using namespace dunedaq;

  appfwk::CommandLineInterpreter args;
  try {
    args = appfwk::CommandLineInterpreter::parse(argc, argv);
  } catch (bpo::error const& e) {
    // Die but do it gracefully gracefully.
    std::cerr << "Failed to interpret command line: " << e.what(); // NOLINT
    exit(1);
  }

  if (args.help_requested) {
    exit(0);
  }

  // Enable DUNE-DAQ logging, including TLOG and ERS messages
  dunedaq::logging::Logging().setup(args.session_name, args.app_name);

  // Create the Application
  appfwk::Application app(args.app_name,
                          args.session_name,
                          args.command_facility_plugin_name,
                          args.conf_service_plugin_name,
                          args.configuration_id);

  try {

    app.init();
    app.run(run_marker);
  } catch (ers::Issue& e) {
    ers::fatal(appfwk::ApplicationFailure(ERS_HERE, args.session_name, args.app_name, e));
  }

  TLOG() << "Application " << args.session_name << '.' << args.app_name << " exiting.";
  return 0;
}
