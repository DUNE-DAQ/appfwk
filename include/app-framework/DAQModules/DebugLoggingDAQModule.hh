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

#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_USERMODULES_DEBUGLOGGINGUSERMODULE_HH_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_USERMODULES_DEBUGLOGGINGUSERMODULE_HH_

#include "app-framework-base/DAQModules/DAQModuleI.hh"

#include <future>
#include <string>

namespace appframework {
/**
 * @brief DebugLoggingDAQModule logs that it has received a command from
 * DAQProcess
 */
class DebugLoggingDAQModule : public DAQModule {
public:
  DebugLoggingDAQModule(std::string name,
                         std::vector<std::shared_ptr<QueueI>> inputs,
                         std::vector<std::shared_ptr<QueueI>> outputs)
    : DAQModule(name, inputs, outputs)
  {}

  /**
   * @brief Logs the reception of the command
   * @param cmd Command from DAQProcess
   */
  std::future<std::string> execute_command(std::string cmd) override;
};
} // namespace appframework

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_USERMODULES_DEBUGLOGGINGUSERMODULE_HH_
