/**
 * @file CommandLineInterpreter helper class
 *
 * CommandLineInterpreter takes the command-line arguments and produces a
 * configuration object for use by DAQProcess
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_APPS_COMMANDLINEINTERPRETER_HH
#define APP_FRAMEWORK_APPS_COMMANDLINEINTERPRETER_HH

#include <boost/program_options.hpp>
namespace bpo = boost::program_options;

#include "TRACE/trace.h"

namespace appframework {
struct CommandLineInterpreter
{
public:
  explicit CommandLineInterpreter()
    : isValid(false)
  {}

  static CommandLineInterpreter ParseCommandLineArguments(int argc, char** argv)
  {
    CommandLineInterpreter output;

    std::ostringstream descstr;
    descstr
      << argv[0]
      << " known arguments (additional arguments will be stored and passed on)";
    bpo::options_description desc(descstr.str());
    desc.add_options()("commandFacility,c",
                       bpo::value<std::string>(),
                       "CommandFacility plugin name")(
      "configManager,m",
      bpo::value<std::string>(),
      "ConfigurationManager plugin name")(
      "service,s",
      bpo::value<std::vector<std::string>>(),
      "Service plugin(s) to load")("configJson,j",
                                   bpo::value<std::string>(),
                                   "JSON Application configuration file name")(
      "help,h", "produce help message");
    bpo::variables_map vm;
    try {
      auto parsed = bpo::command_line_parser(argc, argv)
                      .options(desc)
                      .allow_unregistered()
                      .run();

      output.otherOptions =
        bpo::collect_unrecognized(parsed.options, bpo::include_positional);
      bpo::store(parsed, vm);
      bpo::notify(vm);
    } catch (bpo::error const& e) {
      TLOG_ERROR("CommandLineInterpreter")
        << "Exception from command line processing in " << argv[0] << ": "
        << e.what() << "\n";
      exit(-1);
    }

    if (vm.count("help")) {
      std::cout << desc;
      exit(0);
    }

    if (vm.count("commandFacility")) {
      output.commandFacilityPluginName =
        vm["commandFacility"].as<std::string>();
    } else {
      TLOG_ERROR("CommandLineInterpreter")
        << "CommandFacility not specified on command line! Exiting";
      std::cout << desc;
      exit(-2);
    }
    if (vm.count("configManager")) {
      output.configurationManagerPluginName =
        vm["configManager"].as<std::string>();
    }
    if (vm.count("service")) {
      output.servicePluginNames = vm["service"].as<std::vector<std::string>>();
    }
    if (vm.count("configJson")) {
      output.applicaitonConfigurationFile = vm["configJson"].as<std::string>();
    }
    output.isValid = true;
    return output;
  }

  bool isValid;
  std::string applicaitonConfigurationFile;
  std::string commandFacilityPluginName;
  std::string configurationManagerPluginName;
  std::vector<std::string> servicePluginNames;
  std::vector<std::string> otherOptions;
};
} // namespace appframework

#endif // APP_FRAMEWORK_APPS_COMMANDLINEINTERPRETER_HH