/**
 * @file The DebugLoggingDAQModule class interface
 *
 * DebugLoggingDAQModule is a simple DAQModule implementation that simply logs
 * the fact that it received a command from DAQProcess.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQMODULES_DEBUGLOGGINGDAQMODULE_HH_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQMODULES_DEBUGLOGGINGDAQMODULE_HH_

#include "app-framework/DAQModules/DAQModuleI.hh"

#include <string>
#include <vector>

namespace appframework {
/**
 * @brief DebugLoggingDAQModule logs that it has received a command from
 * DAQProcess
 */
class DebugLoggingDAQModule : public DAQModuleI
{
public:
  DebugLoggingDAQModule(std::string name)
    : DAQModuleI(name)
  {}

  /**
   * @brief Logs the reception of the command
   * @param cmd Command from DAQProcess
   */
  void execute_command(const std::string& cmd,
                       const std::vector<std::string>& args = {}) override;
};
} // namespace appframework

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQMODULES_DEBUGLOGGINGDAQMODULE_HH_
