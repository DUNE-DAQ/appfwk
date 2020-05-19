/**
 * @file UserModule Class Interface
 *
 * The UserModule interface defines the required functionality for all
 * UserModules that use the Application Framework. UserModules are defined as "a
 * set of code which performs a specific task". They are connected to one
 * another within a DAQ Application by Buffer instances, in a graph defined by a
 * ModuleList.
 *
 * This interface is intended to define only absolutely necessary methods to be
 * able to support the many different tasks that UserModules will be asked to
 * perform.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_USERMODULES_USERMODULE_HH_
#define APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_USERMODULES_USERMODULE_HH_

#include <future>
#include <string>

#include "cetlib/BasicPluginFactory.h"
#include "cetlib/compiler_macros.h"

#include "app-framework-base/Buffers/Buffer.hh"

#ifndef EXTERN_C_FUNC_DECLARE_START
#define EXTERN_C_FUNC_DECLARE_START                                            \
  extern "C"                                                                   \
  {
#endif

#define DEFINE_DUNE_USER_MODULE(klass)                                         \
  EXTERN_C_FUNC_DECLARE_START                                                  \
  std::unique_ptr<appframework::UserModule> make(                              \
    std::string n,                                                             \
    std::vector<std::shared_ptr<appframework::BufferI>> i,                     \
    std::vector<std::shared_ptr<appframework::BufferI>> o)                     \
  {                                                                            \
    return std::unique_ptr<appframework::UserModule>(new klass(n, i, o));      \
  }                                                                            \
  }

namespace appframework {
/**
 * @brief The UserModule class is a set of code which performs a specific task.
 *
 * This interface defines the basic methods which all UserModules should expose.
 * Developers implementing UserModules should feel free to use whatever Plugins
 * and Services are necessary to accomplish their needed functionality.
 */
class UserModule {
public:
  UserModule(std::string name,
             std::vector<std::shared_ptr<BufferI>> inputs,
             std::vector<std::shared_ptr<BufferI>> outputs)
    : instance_name_(name)
    , inputs_(inputs)
    , outputs_(outputs)
  {}

  /**
   * @brief Execute a command in this UserModule
   * @param cmd The command from CCM
   * @return String with detailed status of the command (future).
   *
   * execute_command is the single entry point for DAQProcess to pass CCM
   * commands to UserModules. The implementation of this function should route
   * accepted commands to the appropriate functions within the UserModule and
   * return their result. Non-accepted commands should return a status
   * indicating this result.
   */
  virtual std::future<std::string> execute_command(std::string cmd) = 0;

protected:
  std::string instance_name_;
  std::vector<std::shared_ptr<BufferI>> inputs_;
  std::vector<std::shared_ptr<BufferI>> outputs_;
};

inline std::unique_ptr<UserModule>
makeModule(std::string const& module_name,
           std::string const& instance_name,
           std::vector<std::shared_ptr<BufferI>> inputs,
           std::vector<std::shared_ptr<BufferI>> outputs)
{
  static cet::BasicPluginFactory bpf("duneUserModule", "make");

  return bpf.makePlugin<std::unique_ptr<UserModule>>(
    module_name, instance_name, inputs, outputs);
}
} // namespace appframework

#endif // APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_USERMODULES_USERMODULE_HH_
