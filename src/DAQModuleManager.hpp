/**
 * @file DAQModuleManager.hpp Loads and distributes commands to DAQModules
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_SRC_DAQMODULEMANAGER_HPP_
#define APPFWK_SRC_DAQMODULEMANAGER_HPP_

#include "ers/Issue.hpp"
#include "nlohmann/json.hpp"

#include "appfwk/ConfigurationManager.hpp"
#include "appfwk/DAQModule.hpp"
#include "appfwk/ValidationReport.hpp"
#include "conffwk/Configuration.hpp"
#include "confmodel/DaqModule.hpp"

#include "cmdlib/cmd/Structs.hpp"
#include "logging/Logging.hpp" // NOTE: if ISSUES ARE DECLARED BEFORE include logging/Logging.hpp, TLOG_DEBUG<<issue wont work.
#include "opmonlib/OpMonManager.hpp"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace dunedaq {

// Disable coverage collection LCOV_EXCL_START
ERS_DECLARE_ISSUE(appfwk,
                  DAQModuleManagerNotInitialized,
                  "Command " << cmdid << " received before initialization",
                  ((std::string)cmdid))

ERS_DECLARE_ISSUE(appfwk,
                  DAQModuleManagerAlreadyInitialized,
                  "\"init\" Command received when already initialized",
                  ERS_EMPTY)

ERS_DECLARE_ISSUE(appfwk,
                  CommandDispatchingFailed,
                  "Command " << cmdid << " was not executed correctly by: " << modules,
                  ((std::string)cmdid)((std::string)modules))

ERS_DECLARE_ISSUE(appfwk,
                  ConflictingCommandMatching,
                  "Command " << cmdid << " matches multiple times modules: " << modules,
                  ((std::string)cmdid)((std::string)modules))

ERS_DECLARE_ISSUE(appfwk, FailedInfoGathering, "Info gathering failed for module: " << module, ((std::string)module))

ERS_DECLARE_ISSUE_BASE(appfwk,
                       ExceptionWhileInfoGathering,
                       FailedInfoGathering,
                       module << " threw exception while info gathering: " << message,
                       ((std::string)module),
                       ((std::string)message))

ERS_DECLARE_ISSUE(appfwk,
                  ActionPlanNotFound,
                  "No action plan found for command " << cmd << ", taking the following action: " << message,
                  ((std::string)cmd)((std::string)message))

// Re-enable coverage collection LCOV_EXCL_STOP

namespace appfwk {

constexpr int ACTION_PLANS_REQUIRED = 0;
constexpr int ACTION_PLANS_REQUIRED_WARNING = 0;

class DAQModuleManager
{
public:
  explicit DAQModuleManager(const std::string& session_name);

  void initialize(std::shared_ptr<ConfigurationManager> mgr, opmonlib::OpMonManager&);
  bool initialized() const { return m_initialized; }
  void cleanup();
  void reload(opmonlib::OpMonManager&);

  // Execute a properly structured command
  void execute(const std::string& cmd, const DAQModule::CommandData_t& cmd_data);

  // Used during ActionPlan validation
  void set_config_mgr(std::shared_ptr<ConfigurationManager> mgr) { m_configuration_mgr = mgr; }
  void construct_modules(const std::vector<const dunedaq::confmodel::DaqModule*>& modules);
  std::vector<ValidationReport> validate_action_plans(bool throw_on_fatal = true);

private:
  typedef std::map<std::string, std::shared_ptr<DAQModule>> DAQModuleMap_t; ///< DAQModules indexed by name

  void init_modules(const std::vector<const dunedaq::confmodel::DaqModule*>& modules, opmonlib::OpMonManager&);

  void check_command_data(const std::string& id, const DAQModule::CommandData_t& cmd_data);
  DAQModule::CommandData_t get_command_data_for_module(const std::string& mod_name,
                                                       const DAQModule::CommandData_t& cmd_data);
  bool execute_action(const std::string& mod_name, const std::string& action, const DAQModule::CommandData_t& data_obj);
  void execute_action_plan_step(const std::string& cmd,
                                const confmodel::DaqModulesGroup* step,
                                const DAQModule::CommandData_t& cmd_data,
                                bool execution_mode_is_serial);

  std::optional<ValidationReport> check_mod_has_cmd(const std::string& cmd,
                                                  const std::string& mod_class,
                                                  bool is_optional,
                                                  const std::string& mod_id,
                                                  bool throw_on_fatal);

  std::vector<std::string> get_modnames_by_cmdid(cmdlib::cmd::CmdId id);
  std::shared_ptr<ConfigurationManager> m_configuration_mgr;

  std::string m_session_name;
  bool m_initialized;

  DAQModuleMap_t m_module_map;
  std::map<std::string, std::vector<std::string>> m_modules_by_type;
};

} // namespace appfwk
} // namespace dunedaq

#endif // APPFWK_SRC_DAQMODULEMANAGER_HPP_
