/**
 * @file UserModule Class Interface
 *
 * The UserModule interface defines the required functionality for all UserModules that use the Application Framework.
 * UserModules are defined as "a set of code which performs a specific task". They are connected to one another within
 * a DAQ Application by Buffer instances, in a graph defined by a ModuleList.
 *
 * This interface is intended to define only absolutely necessary methods to be able to support the many different tasks that
 * UserModules will be asked to perform.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#ifndef app_framework_base_UserModules_UserModule_hh
#define app_framework_base_UserModules_UserModule_hh

#include <string>
#include <future>

namespace appframework {
/**
 * @brief The UserModule class is a set of code which performs a specific task.
 *
 * This interface defines the basic methods which all UserModules should expose. Developers implementing UserModules
 * should feel free to use whatever Plugins and Services are necessary to accomplish their needed functionality.
 */
class UserModule {
   public:
    /**
     * @brief Execute a command in this UserModule
     * @param cmd The command from CCM
     * @return String with detailed status of the command (future).
     *
     * execute_command is the single entry point for DAQProcess to pass CCM commands to UserModules.
     * The implementation of this function should route accepted commands to the appropriate functions
     * within the UserModule and return their result. Non-accepted commands should return a status indicating
     * this result.
     */
    virtual std::future<std::string> execute_command(std::string cmd) = 0;
};
}  // namespace appframework

#endif  // app_framework_base_UserModules_UserModule_hh