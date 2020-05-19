/**
 * @file DebugLoggingUserModule class implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "app-framework/UserModules/DebugLoggingUserModule.hh"

#include "app-framework-base/Services/Logger.hh"
#include "app-framework-base/UserModules/UserModule.hh"

#include "TRACE/trace.h"

#include <iostream>

namespace appframework {
std::future<std::string>
DebugLoggingUserModule::execute_command(std::string cmd) {
  TLOG(TLVL_INFO) << "Executing command: " << cmd;
  return std::async([]() { return std::string("Success"); });
}
} // namespace appframework

DEFINE_DUNE_USER_MODULE(appframework::DebugLoggingUserModule)