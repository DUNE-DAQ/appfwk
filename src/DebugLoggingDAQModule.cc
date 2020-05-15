/**
 * @file DebugLoggingDAQModule class implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "app-framework/DebugLoggingDAQModule.hh"

#include "app-framework-base/Services/Logger.hh"

#include "TRACE/trace.h"

#include <iostream>

namespace appframework {

  void DebugLoggingDAQModule::execute_command(const std::string & cmd, const std::vector<std::string> & args ) {
      TLOG(TLVL_INFO) << "Executing command: " << cmd << " with " << args.size() << " args" ;
         for ( auto & a : args ) {
             TLOG(TLVL_INFO) << a ; 
	 }
      return ;
  }
} // namespace appframework