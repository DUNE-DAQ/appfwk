/**
 * @file DAQModule Class Interface
 *
 * The DAQModule interface defines the required functionality for all
 * DAQModules that use the Application Framework. DAQModules are defined as "a
 * set of code which performs a specific task". They are connected to one
 * another within a DAQ Application by Queue instances, in a graph defined by a
 * ModuleList.
 *
 * This interface is intended to define only absolutely necessary methods to be
 * able to support the many different tasks that DAQModules will be asked to
 * perform.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_DAQMODULES_DAQMODULEI_HH_
#define APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_DAQMODULES_DAQMODULEI_HH_

#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "app-framework-base/NamedObject.hh"

#include "app-framework-base/DAQModules/DAQModuleIssues.hh"

// for convenience
using json = nlohmann::json;

namespace appframework {
/**
 * @brief The DAQModuleI class implementations are a set of code which performs
 * a specific task.
 *
 * This interface defines the basic methods which all DAQModules should expose.
 * Developers implementing DAQModules should feel free to use whatever Plugins
 * and Services are necessary to accomplish their needed functionality.
 */
class DAQModuleI : public NamedObject
{
public:
  DAQModuleI(std::string name)
    : NamedObject(name)
  {}

  void configure(json config) { configuration_ = config; }

  /**
   * @brief Execute a command in this DAQModuleI
   * @param cmd The command from CCM
   * @return String with detailed status of the command (future).
   *
   * execute_command is the single entry point for DAQProcess to pass CCM
   * commands to DAQModules. The implementation of this function should route
   * accepted commands to the appropriate functions within the DAQModuleI.
   *  Non-accepted commands or failure should return an std::exception
   * indicating this result.
   */
  virtual void execute_command(const std::string& cmd,
                               const std::vector<std::string>& args = {}) = 0;

protected:
  json configuration_;
};

inline std::shared_ptr<DAQModuleI>
makeModule(std::string const& plugin_name, std::string const& instance_name)
{
  auto namedObject = makeNamedObject(plugin_name, instance_name);
  return std::dynamic_pointer_cast<DAQModuleI>(namedObject);
}

} // namespace appframework

#endif // APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_DAQMODULES_DAQMODULEI_HH_
