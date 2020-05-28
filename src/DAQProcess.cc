/**
 * @file DAQProcess class implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "app-framework/DAQProcess.hh"
#include "app-framework/DAQProcessIssues.hh"

#include "app-framework-base/Services/CommandFacility.hh"
#include "app-framework-base/Services/ConfigurationManager.hh"
#include "app-framework-base/Services/Logger.hh"
#include "app-framework-base/Services/ServiceManager.hh"

#include "TRACE/trace.h"
#define TRACE_NAME "DAQProcess" // NOLINT

#include "ers/ers.h" 

#include <memory>
#include <unordered_set>

namespace appframework {
std::unique_ptr<ServiceManager> ServiceManager::handle_ = nullptr;
std::unique_ptr<ConfigurationManager> ConfigurationManager::handle_ = nullptr;

DAQProcess::DAQProcess(std::list<std::string> args)
{
  Logger::setup(args);
  CommandFacility::setup(args);
  ConfigurationManager::setup(args);
  ServiceManager::setup(args);
}

void
DAQProcess::register_modules(ModuleList& ml)
{
  ml.ConstructGraph(queueMap_, daqModuleMap_, commandOrderMap_);
}

void
DAQProcess::execute_command(std::string cmd)
{
  std::unordered_set<std::string> daq_module_list;
  for (auto const& dm : daqModuleMap_) {
    daq_module_list.insert(dm.first);
  }

  TLOG(TLVL_DEBUG) << "Executing Command " << cmd
                   << " for DAQModules defined in the CommandOrderMap";

  if (commandOrderMap_.count(cmd)) {
    for (auto& moduleName : commandOrderMap_[cmd]) {
      if (daqModuleMap_.count(moduleName)) { 
        
	call_command_on_module( *daqModuleMap_[moduleName], cmd );
     
	daq_module_list.erase(moduleName);
      }
    }
  } else {

    ers::warning ( DAQIssues::CommandOrderNotSpecified( ERS_HERE, cmd ) ) ;

  }

  TLOG(TLVL_DEBUG) << "Executing Command " << cmd
                   << " for all remaining DAQModules";
  for (auto const& moduleName : daq_module_list) {

    call_command_on_module( *daqModuleMap_[moduleName], cmd );
  }
}

int
DAQProcess::listen()
{
  return CommandFacility::handle().listen(this);
}

  void DAQProcess::call_command_on_module( DAQModuleI & mod, const std::string & cmd ) {

    try {
      mod.execute_command( cmd ) ;
    }
    catch ( DAQIssues::GeneralDAQModuleIssue & ex ) {
      ers::error( ex ) ;
    }
    // catch (...) {
    //   ers::error( DAQIssues::ModuleThrowUnknown( ERS_HERE, mod.Name(), cmd ) ;
    // }
  } 
} // namespace appframework
