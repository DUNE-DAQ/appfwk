/**
 * @file echo_test_app.cxx
 *
 * echo_test_app shows the basic functionality of DAQProcess by loading a
 * DummyModule
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "DummyModule.hpp"
#include "appfwk/CommandLineInterpreter.hpp"
#include "appfwk/DAQProcess.hpp"

namespace dunedaq::appfwk {
/**
 * @brief ModuleList for the echo_test_app
 */
class dummy_test_app_contructor : public GraphConstructor
{
  // Inherited via ModuleList
  void ConstructGraph(DAQModuleMap& user_module_map, CommandOrderMap& /*command_order_map*/) const override
  {
    user_module_map["dummy"].reset(new DummyModule("test_dummy"));
  }
};
} // namespace dunedaq::appfwk

/**
 * @brief echo_test_app main entry point
 * @param argc Number of arguments
 * @param argv Arguments
 * @return Status code from DAQProcess::listen
 */
int
main(int argc, char* argv[])
{

  using namespace dunedaq::appfwk;

  auto args = CommandLineInterpreter::parse(argc, argv);

  DAQProcess theDAQProcess(args);

  dummy_test_app_contructor gc;
  theDAQProcess.register_modules(gc);

  return theDAQProcess.listen();
}
