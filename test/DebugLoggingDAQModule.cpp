/**
 * @file DebugLoggingDAQModule.cpp DebugLoggingDAQModule class implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "DebugLoggingDAQModule.hpp"

#include "appfwk/Logger.hpp"
#define TRACE_NAME "DebugLoggingDAQModule"


namespace dunedaq::appfwk {
void
DebugLoggingDAQModule::execute_command(const std::string& cmd,
                                       const std::vector<std::string>& args)
{
	ERS_DEBUG(0) << get_name() << ": Executing command: " << cmd << " with "
                  << args.size() << " args";
  for (auto& a : args) {
	  ERS_DEBUG(0) << a;
  }
  return;
}
} // namespace dunedaq::appfwk

DEFINE_DUNE_DAQ_MODULE(dunedaq::appfwk::DebugLoggingDAQModule)
