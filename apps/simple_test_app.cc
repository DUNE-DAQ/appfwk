/**
 * @file simple_test_app to show basic functionality of DAQProcess
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "app-framework/CommandLineInterpreter.hh"
#include "app-framework/DAQProcess.hh"
#include "app-framework/DAQModules/DebugLoggingDAQModule.hh"

namespace appframework {

class simple_test_app_ModuleList : public ModuleList
{
  // Inherited via ModuleList
  void ConstructGraph(QueueMap& buffer_map,
                      DAQModuleMap& user_module_map,
                      CommandOrderMap& command_order_map) override
  {
    user_module_map["debugLogger"].reset(
      new DebugLoggingDAQModule("debugLogger", {}, {}));
  }
};
} // namespace appframework

int
main(int argc, char* argv[])
{

  auto args =
    appframework::CommandLineInterpreter::ParseCommandLineArguments(argc, argv);

  appframework::DAQProcess theDAQProcess(args);

  appframework::simple_test_app_ModuleList ml;
  theDAQProcess.register_modules(ml);

  return theDAQProcess.listen();
}
