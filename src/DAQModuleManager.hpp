/**
 * @file DAQModuleManager.hpp Loads and distributes commands to DAQModules
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_DAQMODULEMANAGER_HPP_
#define APPFWK_INCLUDE_APPFWK_DAQMODULEMANAGER_HPP_

#include "ers/Issue.hpp"
#include "nlohmann/json.hpp"
#include "opmonlib/InfoCollector.hpp"

#include "appfwk/ConfigurationManager.hpp"
#include "appfwk/ModuleConfiguration.hpp"
#include "coredal/DaqModule.hpp"
#include "coredal/ActionStep.hpp"
#include "oksdbinterfaces/Configuration.hpp"

#include "appfwk/app/Structs.hpp"
#include "cmdlib/cmd/Structs.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace dunedaq {

// Disable coverage collection LCOV_EXCL_START
ERS_DECLARE_ISSUE(appfwk,                                                   ///< Namespace
                  DAQModuleManagerNotInitialized,                           ///< Issue class name
                  "Command " << cmdid << " received before initialization", ///< Message
                  ((std::string)cmdid)                                      ///< Message parameters
)
ERS_DECLARE_ISSUE(appfwk,                             ///< Namespace
                  DAQModuleManagerAlreadyInitialized, ///< Issue class name
                  "\"init\" Command received when already initialized",
                  ERS_EMPTY) ///< Message

ERS_DECLARE_ISSUE(appfwk,                                                               ///< Namespace
                  CommandDispatchingFailed,                                             ///< Issue class name
                  "Command " << cmdid << " was not executed correctly by: " << modules, ///< Message
                  ((std::string)cmdid)                                                  ///< Message parameters
                  ((std::string)modules)                                                ///< Message parameters
)

ERS_DECLARE_ISSUE(appfwk,                                                                ///< Namespace
                  ConflictingCommandMatching,                                            ///< Issue class name
                  "Command " << cmdid << " matches multiple times modules: " << modules, ///< Message
                  ((std::string)cmdid)                                                   ///< Message parameters
                  ((std::string)modules)                                                 ///< Message parameters
)

ERS_DECLARE_ISSUE(appfwk,                                                                ///< Namespace
                  FailedInfoGathering,                                                   ///< Issue class name
                  "Info gathering failed for module: " << module,                        ///< Message
                  ((std::string)module)                                                  ///< Message parameters
)

ERS_DECLARE_ISSUE_BASE(appfwk,                                                           ///< Namespace  
		       ExceptionWhileInfoGathering,                                      ///< Issue class name
		       FailedInfoGathering,                                              ///< Base Issue class name
		       module << " threw exception while info gathering: " << message,   ///< Message
		       ((std::string)module),                                            ///< Base Issue params
		       ((std::string)message)                                            ///< This class params
)

ERS_DECLARE_ISSUE(appfwk, ActionPlanNotFound, "No action plan found for command " << cmd, ((std::string)cmd))

ERS_DECLARE_ISSUE(appfwk,
                  ActionPlanValidationFailed,
                  "Error validating action plan " << cmd << ", module " << module << ": " << message,
                  ((std::string)cmd)((std::string)module)((std::string)message)
)
// Re-enable coverage collection LCOV_EXCL_STOP

namespace appfwk {

class DAQModule;

class DAQModuleManager
{
public:
  using dataobj_t = nlohmann::json;

  DAQModuleManager();

  void initialize(std::shared_ptr<ConfigurationManager> mgr);
  bool initialized() const { return m_initialized; }
  void cleanup();

  // Execute a properly structured command
  void execute(const std::string& cmd, const dataobj_t& cmd_data);

  // Gather statistics from modules
  void gather_stats(opmonlib::InfoCollector& ic, int level);

private:
  typedef std::map<std::string, std::shared_ptr<DAQModule>> DAQModuleMap_t; ///< DAQModules indexed by name

  void init_modules(const std::vector<const dunedaq::coredal::DaqModule*>& modules);

  void check_cmd_data(const std::string& id, const dataobj_t& cmd_data);
  dataobj_t get_dataobj_for_module(const std::string& mod_name, const dataobj_t& cmd_data);
  bool execute_action(const std::string& mod_name, const std::string& action, const dataobj_t& data_obj);
  void execute_action_plan_step(const std::string& cmd, const coredal::ActionStep* step, const dataobj_t& cmd_data);

  std::vector<std::string> get_modnames_by_cmdid(cmdlib::cmd::CmdId id);
  std::shared_ptr<ModuleConfiguration> m_module_configuration;

  bool m_initialized;

  DAQModuleMap_t m_module_map;
};

} // namespace appfwk
} // namespace dunedaq

#endif // APPFWK_INCLUDE_APPFWK_DAQMODULEMANAGER_HPP_
