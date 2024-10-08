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

  dunedaq::logging::Logging().setup();

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
  } catch (ers::Issue& e) {
    // Die but do it gracefully gracefully.
    ers::error(appfwk::BadCliUsage(ERS_HERE, e.message()));
    exit(-1);
  }

  if (args.help_requested) {
    exit(0);
  }

  // Get the application and session name from the environment.
  std::string app_name = "";
  std::string session_name = "";

  char* app_name_c = getenv("DUNEDAQ_APPLICATION_NAME");
  char* session_name_c = getenv("DUNEDAQ_SESSION");

  bool missing_env_var =
    !app_name_c || std::string(app_name_c) == "" || !session_name_c || std::string(session_name_c) == "";
  if (missing_env_var) {
    ers::error(appfwk::EnvironmentVariableNotFound(ERS_HERE, "DUNEDAQ_APPLICATION_NAME or DUNEDAQ_SESSION"));
    exit(1);
  }

  app_name = app_name_c;
  session_name = session_name_c;

  if (args.app_name != app_name || args.session_name != session_name) {
    ers::error(appfwk::MismatchedEnvAndCLI(ERS_HERE, "name", "DUNEDAQ_APPLICATION_NAME", args.app_name, app_name));
    ers::error(appfwk::MismatchedEnvAndCLI(ERS_HERE, "session", "DUNEDAQ_SESSION", args.session_name, session_name));
    exit(1);
  }

  // Create the Application
  appfwk::Application app(app_name, session_name, args.command_facility_plugin_name, args.conf_service_plugin_name);

  app.init();
  app.run(run_marker);

  TLOG() << "Application " << app_name << " exiting.";
  return 0;
}
