/**
 * @file echo_test_app.cc
 *
 * echo_test_app shows the basic functionality of DAQProcess by loading a
 * DebugLoggingDAQModule
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "DebugLoggingDAQModule.hh"
#include "app-framework/CommandLineInterpreter.hh"
#include "app-framework/DAQProcess.hh"

namespace appframework {
/**
 * @brief ModuleList for the echo_test_app
*/
class echo_test_app_ModuleList : public ModuleList
{
  // Inherited via ModuleList
  void ConstructGraph(DAQModuleMap& user_module_map,
                      CommandOrderMap& command_order_map) override
  {
    user_module_map["debugLogger"].reset(
      new DebugLoggingDAQModule("debugLogger"));
  }
};
} // namespace appframework

/**
 * @brief echo_test_app main entry point
 * @param argc Number of arguments
 * @param argv Arguments
 * @return Status code from DAQProcess::listen
*/
int
main(int argc, char* argv[])
{
  auto args =
    appframework::CommandLineInterpreter::ParseCommandLineArguments(argc, argv);

  appframework::DAQProcess theDAQProcess(args);

  appframework::echo_test_app_ModuleList ml;
  theDAQProcess.register_modules(ml);

  return theDAQProcess.listen();
}
