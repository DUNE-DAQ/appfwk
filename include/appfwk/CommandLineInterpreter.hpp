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

ERS_DECLARE_ISSUE(appfwk,                                                             // Namespace
                  CommandLineIssue,                                                   // Class name
                  "Command-line processing issue in " << app_name << ": " << message, // Message
                  ((std::string)app_name)((std::string)message))                      // Args

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
    desc.add_options()
      ("name,n", bpo::value<std::string>()->required(), "Application name")
      ("partition,p", bpo::value<std::string>()->default_value("global"), "Partition name")
      ("commandFacility,c", bpo::value<std::string>()->required(), "CommandFacility URI")
      ("rest-api,r", bpo::value<std::uint32_t>()->default_value(0), "REST API Port")
      ("informationService,i", bpo::value<std::string>()->default_value("stdout://flat"), "Information Service URI")
      ("help,h", "produce help message");

    bpo::variables_map vm;
    try {
      auto parsed = bpo::command_line_parser(argc, argv).options(desc).allow_unregistered().run();

      output.other_options = bpo::collect_unrecognized(parsed.options, bpo::include_positional);
      bpo::store(parsed, vm);
    } catch (bpo::error const& e) {
      throw CommandLineIssue(ERS_HERE, *argv, e.what());
    }

    if (vm.count("help")) {
      std::cout << desc; // NOLINT
      exit(0);
    }

    try {
      bpo::notify(vm);
    } catch (bpo::error const& e) {
      throw CommandLineIssue(ERS_HERE, *argv, e.what());
    }

    output.app_name = vm["name"].as<std::string>();
    output.partition_name = vm["partition"].as<std::string>();
    output.command_facility_plugin_name = vm["commandFacility"].as<std::string>();
    output.rest_api = vm["rest-api"].as<std::uint32_t>();
    output.info_service_plugin_name = vm["informationService"].as<std::string>();
    output.is_valid = true;
    return output;
  }

  bool is_valid{ false }; ///< Whether the command line was successfully parsed

  std::string app_name;
  std::string partition_name;
  std::string command_facility_plugin_name; ///< Name of the CommandFacility plugin to load
  std::uint32_t rest_api;
  std::string info_service_plugin_name; ///< Name of the InfoService plugin to load

  std::vector<std::string> other_options; ///< Any other options which were passed and not recognized
};
} // namespace appfwk
} // namespace dunedaq

#endif // APPFWK_INCLUDE_APPFWK_COMMANDLINEINTERPRETER_HPP_
