/**
 * @file CommandLineInterpreter.hpp CommandLineInterpreter helper class
 *
 * CommandLineInterpreter takes the command-line arguments and produces a
 * configuration object for use by DAQProcess
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_COMMANDLINEINTERPRETER_HPP_
#define APPFWK_INCLUDE_APPFWK_COMMANDLINEINTERPRETER_HPP_

#include "boost/program_options.hpp"
#include "ers/ers.hpp"

#include <string>
#include <vector>

namespace bpo = boost::program_options;

namespace dunedaq {

// Disable coverage collection LCOV_EXCL_START
ERS_DECLARE_ISSUE(appfwk,                                                             // Namespace
                  CommandLineIssue,                                                   // Class name
                  "Command-line processing issue in " << app_name << ": " << message, // Message
                  ((std::string)app_name)((std::string)message))                      // Args

// Re-enable coverage collection LCOV_EXCL_STOP
namespace appfwk {
/**
 * @brief CommandLineInterpreter parses the command-line options given to the
 * application and stores the results as validated data members
 */
struct CommandLineInterpreter
{
public:
  /**
   * @brief Parse the command line and return a CommandLineInterpreter struct
   * @param argc Number of arguments
   * @param argv Command-line arguments
   * @return CommandLineInterpreter structure with parsed arguments
   */
  static CommandLineInterpreter parse(int argc, char** argv)
  {
    CommandLineInterpreter output;

    std::ostringstream descstr;
    descstr << *argv
            << " known arguments (additional arguments will be stored and "
               "passed on)";
    bpo::options_description desc(descstr.str());
    desc.add_options()("name,n", bpo::value<std::string>()->required(), "Application name")(
      "system,s", bpo::value<std::string>()->default_value("global"), "System name")(
      "commandFacility,c", bpo::value<std::string>()->required(), "CommandFacility URI")(
      "configurationService,d", bpo::value<std::string>()->required(), "Configuration Service URI")(
      "help,h", "produce help message");

    bpo::variables_map vm;
    try {
      auto parsed = bpo::command_line_parser(argc, argv).options(desc).allow_unregistered().run();

      output.other_options = bpo::collect_unrecognized(parsed.options, bpo::include_positional);
      bpo::store(parsed, vm);
    } catch (bpo::error const& e) {
      throw CommandLineIssue(ERS_HERE, *argv, e.what());
    }

    if (vm.count("help")) {
      std::cout << desc << std::endl; // NOLINT
      output.help_requested = true;
      return output;
    }

    try {
      bpo::notify(vm);
    } catch (bpo::error const& e) {
      throw CommandLineIssue(ERS_HERE, *argv, e.what());
    }

    output.app_name = vm["name"].as<std::string>();
    output.system_name = vm["system"].as<std::string>();
    output.command_facility_plugin_name = vm["commandFacility"].as<std::string>();
    output.conf_service_plugin_name = vm["configurationService"].as<std::string>();
    return output;
  }

  bool help_requested{ false }; ///< Did the user just ask for help?

  std::string app_name{ "" };
  std::string system_name{ "" };
  std::string command_facility_plugin_name{ "" }; ///< Name of the CommandFacility plugin to load
  std::string conf_service_plugin_name{ "" };     ///< Name of the ConfService plugin to load

  std::vector<std::string> other_options{}; ///< Any other options which were passed and not recognized
};
} // namespace appfwk
} // namespace dunedaq

#endif // APPFWK_INCLUDE_APPFWK_COMMANDLINEINTERPRETER_HPP_
