/**
 * @file DAQProcess.cpp DAQProcess class implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/DAQProcess.hpp"

#include "appfwk/CommandFacility.hpp"
#include "appfwk/Logger.hpp"

#include "TRACE/trace.h"
/**
 * @brief Name used by TRACE TLOG calls from this source file
 */
#define TRACE_NAME "DAQProcess" // NOLINT

#include "ers/ers.h"

#include <memory>
#include <unordered_set>

namespace dunedaq::appfwk {
std::unique_ptr<CommandFacility> CommandFacility::handle_ = nullptr;

DAQProcess::DAQProcess(CommandLineInterpreter args)
{
  CommandFacility::setHandle(makeCommandFacility(args.commandFacilityPluginName));
  Logger::setup(args.otherOptions);
  CommandFacility::handle().setup(args.otherOptions);
}

void
DAQProcess::register_modules(GraphConstructor& ml)
{
  ml.ConstructGraph(daqModuleMap_, commandOrderMap_);
}

void
DAQProcess::execute_command(std::string const& cmd, std::vector<std::string> const& args)
{
  std::unordered_set<std::string> daq_module_list;
  for (auto const& dm : daqModuleMap_) {
    daq_module_list.insert(dm.first);
  }

  TLOG(TLVL_TRACE) << "Executing Command " << cmd << " for DAQModules defined in the CommandOrderMap";

  if (commandOrderMap_.count(cmd)) {
    for (auto& moduleName : commandOrderMap_[cmd]) {
      if (daqModuleMap_.count(moduleName)) {

        call_command_on_module(*daqModuleMap_[moduleName], cmd, args);

        daq_module_list.erase(moduleName);
      }
    }
  } else {

    ers::warning(CommandOrderNotSpecified(ERS_HERE, cmd));
  }

  TLOG(TLVL_TRACE) << "Executing Command " << cmd << " for all remaining DAQModules";
  for (auto const& moduleName : daq_module_list) {

    call_command_on_module(*daqModuleMap_[moduleName], cmd, args);
  }
}

int
DAQProcess::listen()
{
  return CommandFacility::handle().listen(this);
}

void
DAQProcess::call_command_on_module(DAQModule& mod, const std::string& cmd, std::vector<std::string> const& args)
{

  try {
    mod.execute_command(cmd, args);
  } catch (GeneralDAQModuleIssue& ex) {
    ers::error(ex);
  } catch (...) { // NOLINT
    ers::error(ModuleThrowUnknown(ERS_HERE, mod.get_name(), cmd));
  }
}
} // namespace dunedaq::appfwk
