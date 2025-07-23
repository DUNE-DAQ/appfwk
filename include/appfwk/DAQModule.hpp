/**
 * @file DAQModule.hpp DAQModule Class Interface
 *
 * The DAQModule interface defines the required functionality for all DAQModules that use the Application Framework.
 * DAQModules are defined as "a set of code which performs a specific task". DAQModules are hosted within Applications
 * and communicate with one another via the `iomanager` messaging system.
 *
 * This interface is intended to define only absolutely necessary methods to be able to support the many different tasks
 * that DAQModules will be asked to perform.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_DAQMODULE_HPP_
#define APPFWK_INCLUDE_APPFWK_DAQMODULE_HPP_

#include "appfwk/ConfigurationManager.hpp"

#include "logging/Logging.hpp" // NOTE: if ISSUES ARE DECLARED BEFORE include logging/Logging.hpp, TLOG_DEBUG<<issue wont work.
#include "opmonlib/MonitorableObject.hpp"
#include "utilities/NamedObject.hpp"

#include "cetlib/BasicPluginFactory.h"
#include "cetlib/compiler_macros.h"
#include "ers/Issue.hpp"
#include "nlohmann/json.hpp"

#include <functional>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#ifndef EXTERN_C_FUNC_DECLARE_START
// NOLINTNEXTLINE(build/define_used)
#define EXTERN_C_FUNC_DECLARE_START                                                                                    \
  extern "C"                                                                                                           \
  {
#endif

/**
 * @brief Declare the function that will be called by the plugin loader
 * @param klass Class to be defined as a DUNE DAQ Module
 */
// NOLINTNEXTLINE(build/define_used)
#define DEFINE_DUNE_DAQ_MODULE(klass)                                                                                  \
  EXTERN_C_FUNC_DECLARE_START                                                                                          \
  std::shared_ptr<dunedaq::appfwk::DAQModule> make(std::string n)                                                      \
  {                                                                                                                    \
    return std::shared_ptr<dunedaq::appfwk::DAQModule>(new klass(n));                                                  \
  }                                                                                                                    \
  }

namespace dunedaq {

// Disable coverage collection LCOV_EXCL_START
/**
 * @brief A ERS Issue for DAQModule creation failure
 */
ERS_DECLARE_ISSUE(appfwk,
                  DAQModuleCreationFailed,
                  "Failed to create DAQModule " << instance_name << " of type " << plugin_name,
                  ((std::string)plugin_name)((std::string)instance_name))

/**
 * @brief A generic DAQModule ERS Issue
 */
ERS_DECLARE_ISSUE(appfwk, GeneralDAQModuleIssue, " DAQModule: " << name, ((std::string)name))

/**
 * @brief Initialization failed ERS Issue (used by DAQModuleManager)
 */
ERS_DECLARE_ISSUE_BASE(appfwk,
                       DAQModuleInitFailed,
                       appfwk::GeneralDAQModuleIssue,
                       " init failed.",
                       ((std::string)name),
                       ERS_EMPTY)

/**
 * @brief Generic command ERS Issue
 */
ERS_DECLARE_ISSUE_BASE(appfwk,
                       CommandIssue,
                       appfwk::GeneralDAQModuleIssue,
                       " Command " << cmd,
                       ((std::string)name),
                       ((std::string)cmd))

/**
 * @brief The CommandRegistrationFailed DAQModule ERS Issue
 */
ERS_DECLARE_ISSUE_BASE(appfwk,
                       CommandRegistrationFailed,
                       appfwk::CommandIssue,
                       "Command registration. ",
                       ((std::string)cmd)((std::string)name),
                       ERS_EMPTY)

/**
 * @brief The CommandRegistrationFailedMessage DAQModule ERS Issue
 */
ERS_DECLARE_ISSUE_BASE(appfwk,
                       CommandRegistrationFailedMessage,
                       appfwk::CommandIssue,
                       "Command registration failed: " << message,
                       ((std::string)cmd)((std::string)name),
                       ((std::string)message))

/**
 * @brief The UnknownCommand DAQModule ERS Issue
 */
ERS_DECLARE_ISSUE_BASE(appfwk,
                       UnknownCommand,
                       appfwk::CommandIssue,
                       "Command is not recognised",
                       ((std::string)cmd)((std::string)name),
                       ERS_EMPTY)

/**
 * @brief The CommandFailed DAQModule ERS Issue
 */
ERS_DECLARE_ISSUE_BASE(appfwk,
                       CommandFailed,
                       appfwk::CommandIssue,
                       "Command Failed. Reason " << reason,
                       ((std::string)cmd)((std::string)name),
                       ((std::string)reason))

/**
 * @brief The MissingConnection DAQModule ERS Issue
 */
ERS_DECLARE_ISSUE_BASE(appfwk,
                       MissingConnection,
                       appfwk::GeneralDAQModuleIssue,
                       "Required Connection Not Found. Type: " << type << ", direction: " << direction,
                       ((std::string)name),
                       ((std::string)type)((std::string)direction))

// Re-enable coverage collection LCOV_EXCL_STOP
namespace appfwk {

/**
 * @brief The DAQModule class implementations are a set of code which performs
 * a specific task.
 *
 * This interface defines the basic methods which all DAQModules should expose.
 * Developers implementing DAQModules should feel free to use whatever Plugins
 * and Services are necessary to accomplish their needed functionality.
 *
 * This header also contains the definitions of the Issues that can be
 * thrown by the DAQModule.
 */
class DAQModule
  : public utilities::NamedObject
  , public opmonlib::MonitorableObject
{
public:
  using data_t = nlohmann::json;

  explicit DAQModule(std::string name)
    : utilities::NamedObject(name)
  {
  }

  virtual ~DAQModule() noexcept = default;

  /**
   * @brief Initializes the module
   * @param mcfg Pointer to the ConfigurationManager instance used to retrieve configuration objects
   *
   * Initialisation of the module. Abstract method to be overridden by derived classes.
   */
  virtual void init(std::shared_ptr<ConfigurationManager> mcfg) = 0;

  /**
   * @brief Execute a command in this DAQModule
   * @param name The command from CCM
   * @param data Arguments for the command from CCM
   *
   * execute_command is the single entry point for DAQModuleManager to pass CCM commands to DAQModules. If the given
   * command has not been registered, it will throw an UnknownCommand ERS exception.
   */
  void execute_command(const std::string& name, const data_t& data = {});

  std::vector<std::string> get_commands() const;

  bool has_command(const std::string& name) const;

  void set_command_registration_allowed(bool allowed) { m_command_registration_allowed = allowed; }

protected:
  /**
   * @brief Registers a mdoule command under the name `name`.
   * @param name Name of the command to add to the commands list executable by execute_command
   * @param f Callback function that executes the command logic within the DAQModule
   *
   * This method will throw a CommandRegistrationFailed ERS exception if the command could not be added
   */
  template<typename Child>
  void register_command(const std::string& name, void (Child::*f)(const data_t&));

  DAQModule(DAQModule const&) = delete;
  DAQModule(DAQModule&&) = delete;
  DAQModule& operator=(DAQModule const&) = delete;
  DAQModule& operator=(DAQModule&&) = delete;

private:
  using CommandMap_t = std::map<std::string, std::function<void(const data_t&)>>;
  CommandMap_t m_commands;
  std::atomic<bool> m_command_registration_allowed{ true };
};

std::shared_ptr<DAQModule>
make_module(std::string const& plugin_name, std::string const& instance_name);

} // namespace appfwk

} // namespace dunedaq

#include "detail/DAQModule.hxx"

#endif // APPFWK_INCLUDE_APPFWK_DAQMODULE_HPP_
