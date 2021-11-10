/**
 * @file DAQModule.hpp DAQModule Class Interface
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

#ifndef APPFWK_INCLUDE_APPFWK_DAQMODULE_HPP_
#define APPFWK_INCLUDE_APPFWK_DAQMODULE_HPP_

#include "appfwk/NamedObject.hpp"

#include "opmonlib/InfoCollector.hpp"

#include "cetlib/BasicPluginFactory.h"
#include "cetlib/compiler_macros.h"
#include "ers/Issue.hpp"
#include "nlohmann/json.hpp"

#include <functional>
#include <map>
#include <memory>
#include <string>
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
ERS_DECLARE_ISSUE(appfwk,                  ///< Namespace
                  DAQModuleCreationFailed, ///< Type of the Issue
                  "Failed to create DAQModule " << instance_name << " of type "
                                                << plugin_name,          ///< Log Message from the issue
                  ((std::string)plugin_name)((std::string)instance_name) ///< Message parameters
)

/**
 * @brief A generic DAQModule ERS Issue
 */
ERS_DECLARE_ISSUE(appfwk,                 ///< Namespace
                  GeneralDAQModuleIssue,  ///< Issue class name
                  " DAQModule: " << name, ///< Message
                  ((std::string)name)     ///< Message parameters
)

/**
 * @brief Generic command ERS Issue
 */
ERS_DECLARE_ISSUE_BASE(appfwk,                        ///< Namespace
                       CommandIssue,                  ///< Type of the issue
                       appfwk::GeneralDAQModuleIssue, ///< Base class of the issue
                       " Command " << cmd,            ///< Log Message from the issue
                       ((std::string)name),           ///< Base class attributes
                       ((std::string)cmd)             ///< Attribute of this class
)

/**
 * @brief The CommandFailed DAQModule ERS Issue
 */
ERS_DECLARE_ISSUE_BASE(appfwk,                                ///< Namespace
                       CommandRegistrationFailed,             ///< Type of the Issue
                       appfwk::CommandIssue,                  ///< Base class of the Issue
                       "Command registration failed.",        ///< Log Message from the issue
                       ((std::string)cmd)((std::string)name), ///< Base class attributes
                       ERS_EMPTY                              ///< Attribute of this class
)

/**
 * @brief The UnknownCommand DAQModule ERS Issue
 */
ERS_DECLARE_ISSUE_BASE(appfwk,                                ///< Namespace
                       UnknownCommand,                        ///< Issue class name
                       appfwk::CommandIssue,                  ///< Base class of the issue
                       "Command is not recognised",           ///< Log Message from the issue
                       ((std::string)cmd)((std::string)name), ///< Base class attributes
                       ERS_EMPTY                              ///< Attribute of this class
)

/**
 * @brief The CommandFailed DAQModule ERS Issue
 */
ERS_DECLARE_ISSUE_BASE(appfwk,                                ///< Namespace
                       CommandFailed,                         ///< Type of the Issue
                       appfwk::CommandIssue,                  ///< Base class of the Issue
                       "Command Failed. Reason " << reason,   ///< Log Message from the issue
                       ((std::string)cmd)((std::string)name), ///< Base class attributes
                       ((std::string)reason)                  ///< Attribute of this class
)

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
class DAQModule : public NamedObject
{
public:
  using data_t = nlohmann::json;

  /**
   * @brief DAQModule Constructor
   * @param name Name of the DAQModule
   */
  explicit DAQModule(std::string name)
    : NamedObject(name)
  {}

  /**
   * @brief DAQModule destructor
   */
  virtual ~DAQModule() noexcept = default;

  /**
   * @brief      Initializes the module
   *
   * Initialisation of the module. Abstract method to be overridden by derived classes.
   */
  virtual void init(const data_t&) = 0;

  /**
   * @brief Execute a command in this DAQModule
   * @param cmd The command from CCM
   * @param args Arguments for the command from CCM
   * @return String with detailed status of the command (future).
   *
   * execute_command is the single entry point for DAQProcess to pass CCM
   * commands to DAQModules. The implementation of this function should route
   * accepted commands to the appropriate functions within the DAQModule.
   *  Non-accepted commands or failure should return an ERS exception
   * indicating this result.
   */
  void execute_command(const std::string& name, const data_t& data = {});

  std::vector<std::string> get_commands() const;

  bool has_command(const std::string& name) const;

  virtual void get_info(opmonlib::InfoCollector& /*ci*/, int /*level*/) { return; }

protected:
  /**
   * @brief Registers a mdoule command under the name `cmd`.
   * Returns whether the command was inserted (false meaning that command `cmd` already exists)
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
};

/**
 * @brief Load a DAQModule plugin and return a shared_ptr to the contained
 * DAQModule class
 * @param plugin_name Name of the plugin, e.g. DebugLoggingDAQModule
 * @param instance_name Name of the returned DAQModule instance, e.g.
 * DebugLogger1
 * @return shared_ptr to created DAQModule instance
 */
inline std::shared_ptr<DAQModule>
make_module(std::string const& plugin_name, std::string const& instance_name)
{
  static cet::BasicPluginFactory bpf("duneDAQModule", "make");

  std::shared_ptr<DAQModule> mod_ptr;
  try {
    mod_ptr = bpf.makePlugin<std::shared_ptr<DAQModule>>(plugin_name, instance_name);
  } catch (const cet::exception& cexpt) {
    throw DAQModuleCreationFailed(ERS_HERE, plugin_name, instance_name, cexpt);
  }
  return mod_ptr;
}

} // namespace appfwk

} // namespace dunedaq

#include "detail/DAQModule.hxx"

#endif // APPFWK_INCLUDE_APPFWK_DAQMODULE_HPP_

// Local Variables:
// c-basic-offset: 2
// End:
