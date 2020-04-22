/**
 * @file The DebugLoggingUserModule class interface
 *
 * DebugLoggingUserModule is a simple UserModule implementation that simply logs the fact that
 * it received a command from DAQProcess.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#ifndef APP_FRAMEWORK_DEBUGLOGGINGUSERMODULE_HH
#define APP_FRAMEWORK_DEBUGLOGGINGUSERMODULE_HH

#include "app-framework-base/UserModules/UserModule.hh"

#include <string>
#include <future>

namespace appframework {
/**
 * @brief DebugLoggingUserModule logs that it has received a command from DAQProcess
 */
class DebugLoggingUserModule : public UserModule {
   public:
    /**
     * @brief Logs the reception of the command
     * @param cmd Command from DAQProcess
     */
    std::future<std::string> execute_command(std::string cmd) override;

};
}  // namespace appframework

#endif  // APP_FRAMEWORK_DEBUGLOGGINGUSERMODULE_HH