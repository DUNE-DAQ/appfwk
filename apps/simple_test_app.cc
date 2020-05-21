/**
 * @file simple_test_app to show basic functionality of DAQProcess
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "app-framework/DAQProcess.hh"
#include "app-framework/QueryResponseCommandFacility.hh"
#include "app-framework/UserModules/DebugLoggingUserModule.hh"
#include "app-framework-base/Services/Logger.hh" // ers/trace logging TLOG_ERROR(), etc.

namespace appframework {
std::unique_ptr<CommandFacility> CommandFacility::handle_ =
    std::unique_ptr<CommandFacility>(new QueryResponseCommandFacility());

class simple_test_app_ModuleList : public ModuleList {
  // Inherited via ModuleList
  void ConstructGraph(BufferMap &buffer_map, UserModuleMap &user_module_map,
                      CommandOrderMap &command_order_map) override {
    user_module_map["debugLogger"].reset(new DebugLoggingUserModule());
  }
};
} // namespace appframework

int main(int argc, char *argv[]) {
  std::list<std::string> args;
  for (int ii = 1; ii < argc; ++ii) {
    args.push_back(std::string(argv[ii]));
  }

  //ERS_LOG( "Hello there, I've been summoned with " << argc << " arguments" );
  TLOG(TLVL_LOG) << "Hello there, I've been summoned with " << argc << " arguments";

  appframework::DAQProcess theDAQProcess(args);

  appframework::simple_test_app_ModuleList ml;
  theDAQProcess.register_modules(ml);

  return theDAQProcess.listen();
}
