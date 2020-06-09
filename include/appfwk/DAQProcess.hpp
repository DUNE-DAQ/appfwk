/**
 * @file DAQProcess.hpp DAQProcess class declaration
 *
 * DAQProcess is the central container for instantiated DAQModules and Queues
 * within a DAQ Application. It loads a ModuleList which defines the graph of
 * DAQModules and Queues and any command ordering. DAQProcess is responsible
 * for distributing commands recieved from CCM to the DAQModules in the order
 * defined.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQPROCESS_HPP_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQPROCESS_HPP_

#include "appfwk/GraphConstructor.hpp"
#include "appfwk/DAQModule.hpp"
#include "appfwk/CommandLineInterpreter.hpp"

#include <string>

namespace dunedaq::appfwk {
/**
 * @brief The DAQProcess class is the central container for DAQModules and
 * Queues.
 *
 * DAQProcess receives commands from CCM and distributes them to the DAQModules
 * in the order defined in the CommandOrderMap received from the ModuleList
 * during register_modules.
 */
class DAQProcess
{
public:
  /**
   * @brief DAQProcess Constructor
   * @param args Command-line arguments to the DAQ Application
   *
   * The DAQProcess constructor instantiates essential DAQ Application services.
   * Services are passed the command-line options and may also read basic
   * configuration from the environment.
   */
  explicit DAQProcess(CommandLineInterpreter args);
  /**
   * @brief Using the given ModuleList, construct the graph of DAQModules and
   * Queues
   * @param ml ModuleList to call ModuleList::ConstructGraph on
   *
   * The register_modules function calls the ModuleList's ConstructGraph
   * function, which instantiates and links together the DAQModules and Queues
   * needed by this DAQ Application. ConstructGraph also defines any ordering of
   * commands for DAQModules.
   */
  void register_modules( GraphConstructor & gc );
  /**
   * @brief Execute the specified command on the loaded DAQModules
   * @param cmd Command to execute
   * @param args Arguments for the command
   *
   * This function will determine if there is an entry in the command order map
   * for this command, and if so, first send the command to the DAQModules in
   * that list in the order specified. Then, any remaining DAQModules will
   * receive the command in an unspecified order.
   */
  void execute_command(std::string const& cmd,
                       std::vector<std::string> const& args = {});
  /**
   * @brief Start the CommandFacility listener
   * @return Return code from listener
   *
   * This function should call the loaded CommandFacility::listen method, which
   * should block for the duration of the DAQ Application, calling
   * execute_command as necessary.
   */
  int listen();

  DAQProcess(const DAQProcess&) = delete; ///< DAQProcess is not copy-constuctible
  DAQProcess& operator=(const DAQProcess&) = delete; ///< DAQProcess is not copy-assignable
  DAQProcess(DAQProcess&&) = delete; ///< DAQProcess is not move-constructible
  DAQProcess& operator=(DAQProcess&&) = delete; ///< DAQProcess is not move-assignable

protected:
  /**
   * @brief Call a module's execute_command function
   * @param module Module to call execute_command on
   * @param cmd Command name
   * @param args Command arguments
  */
  void call_command_on_module(DAQModule& module, const std::string& cmd, std::vector<std::string> const& args);

private:
  DAQModuleMap daqModuleMap_;       ///< String alias for each DAQModule
  CommandOrderMap commandOrderMap_; ///< Order DAQModule commands by alias
};
} // namespace dunedaq::appfwk

/**
 * @brief DAQ Process generic Issue
 */
ERS_DECLARE_ISSUE(appframework,///< Namespace
                  DAQProcessIssue, ///< Type of the Issue
                  "General DAQProcess Issue",///< Message for Issue
                  ERS_EMPTY)
    
    /**
     * @brief Issue thrown when the ordering for commands is not specified
     */
ERS_DECLARE_ISSUE_BASE(appframework, ///< Namespace
                       CommandOrderNotSpecified, ///< Class Name for Issue
                       DAQProcessIssue, ///< Base class for issue
                       "Command "
                         << cmd
                         << " does not have a specified propagation order ",///< Message for Issue
                       ERS_EMPTY,
                       ((std::string)cmd))

    /**
     * @brief Issue thrown when an unknown exception is thrown from a command
     */
ERS_DECLARE_ISSUE_BASE(appframework, ///< Namespace
                       ModuleThrowUnknown,///< Class Name for Issue
                       DAQProcessIssue,///< Base class for issue
                       "Module " << mod_name
                                 << " threw an unknown exception after command "
                                 << cmd,///< Message for Issue
                       ERS_EMPTY,
                       ((std::string)mod_name)((std::string)cmd))

    /**
     * @brief Issue thrown when a std::exception is thrown from a command
     */
ERS_DECLARE_ISSUE_BASE(appframework, ///< Namespace
                       ModuleThowStd,///< Class Name for Issue
                       DAQProcessIssue,///< Base class for issue
                       "Module " << mod_name
                                 << " threw an std::exception after command "
                                 << cmd, ///< Message for Issue
                       ERS_EMPTY,
                       ((std::string)mod_name)((std::string)cmd))

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQPROCESS_HPP_
