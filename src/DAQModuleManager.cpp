/**
 * @file DAQModuleManager.cpp DAQModuleManager implementataion
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "DAQModuleManager.hpp"

#include "appfwk/DAQModule.hpp"
#include "appfwk/cmd/Nljs.hpp"

#include "cmdlib/cmd/Nljs.hpp"
#include "confmodel/DaqModulesGroup.hpp"
#include "confmodel/DaqModulesGroupById.hpp"
#include "confmodel/DaqModulesGroupByType.hpp"
#include "confmodel/Session.hpp"
#include "iomanager/IOManager.hpp"
#include "logging/Logging.hpp"

#include <future>
#include <map>
#include <memory>
#include <regex>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace dunedaq::appfwk {

DAQModuleManager::DAQModuleManager(const std::string& session_name)
  : m_session_name(session_name)
  , m_initialized(false)
{
}

void
DAQModuleManager::initialize(std::shared_ptr<ConfigurationManager> cfgMgr, opmonlib::OpMonManager& opm)
{
  set_config_mgr(cfgMgr);
  cfgMgr->initialize();
  get_iomanager()->configure(m_session_name,
                             m_configuration_mgr->get_queues(),
                             m_configuration_mgr->get_networkconnections(),
                             m_configuration_mgr->get_connectivity_service(),
                             opm);
  init_modules(m_configuration_mgr->get_modules(), opm);

  validate_action_plans();

  this->m_initialized = true;
}

std::optional<ValidationReport>
DAQModuleManager::check_mod_has_cmd(const std::string& cmd,
                                    const std::string& mod_class,
                                    bool is_optional,
                                    const std::string& mod_id,
                                    bool throw_on_fatal)
{
  std::string app = m_configuration_mgr->get_app_name();

  if (!m_modules_by_type.count(mod_class) || m_modules_by_type[mod_class].size() == 0) {
    if (is_optional) {
      ValidationReport report(ValidationReport::Severity::Ignored,
                           app,
                           mod_class,
                           cmd,
                           "No modules of class " + mod_class + " in application (optional step)");

      return report;
    }
    if (mod_id == "") {
      ValidationReport report(ValidationReport::Severity::Warning,
                           app,
                           mod_class,
                           cmd,
                           "No modules of class " + mod_class + " in application!");
      ers::warning(ActionPlanValidationFailed(ERS_HERE, report.get_command(), report.get_module(), report.get_message()));
      return report;
    } else {
      ValidationReport report(ValidationReport::Severity::Fatal,
                           app,
                           mod_class,
                           cmd,
                           "No modules of class " + mod_class + " in application!");
      if (throw_on_fatal)
        throw ActionPlanValidationFailed(
          ERS_HERE, report.get_command(), report.get_module(), report.get_message());
      else
        ers::error(ActionPlanValidationFailed(
          ERS_HERE, report.get_command(), report.get_module(), report.get_message()));
      return report;
    }
  }

  auto module_test = m_module_map[m_modules_by_type[mod_class][0]];
  if (mod_id != "") {
    bool match = false;
    for (auto& mod_name : m_modules_by_type[mod_class]) {
      if (mod_id == mod_name) {
        module_test = m_module_map[mod_name];
        match = true;
        break;
      }
    }
    if (!match && !is_optional) {
      ValidationReport report(
        ValidationReport::Severity::Fatal, app, mod_class, cmd, "No module with id " + mod_id + " found.");

      if (throw_on_fatal)
        throw ActionPlanValidationFailed(ERS_HERE, report.get_command(), report.get_module(), report.get_message());
      else
        ers::error(
          ActionPlanValidationFailed(ERS_HERE, report.get_command(), report.get_module(), report.get_message()));
      return report;
    }
  }

  if (!module_test->has_command(cmd)) {
    ValidationReport report(
      ValidationReport::Severity::Fatal, app, mod_class, cmd, "Module does not have command " + cmd + " registered.");

    if (throw_on_fatal)
      throw ActionPlanValidationFailed(ERS_HERE, report.get_command(), report.get_module(), report.get_message());
    else
      ers::error(ActionPlanValidationFailed(ERS_HERE, report.get_command(), report.get_module(), report.get_message()));
    return report;
  }
  return {};
}

void
DAQModuleManager::construct_modules(const std::vector<const dunedaq::confmodel::DaqModule*>& modules)
{
  for (const auto mod : modules) {
    TLOG_DEBUG(0) << "construct: " << mod->class_name() << " : " << mod->UID();
    auto mptr = make_module(mod->class_name(), mod->UID());
    // Once constructed, DAQModules should not try to regsiter any more commands
    mptr->set_command_registration_allowed(false);
    m_module_map.emplace(mod->UID(), mptr);

    if (!m_modules_by_type.count(mod->class_name())) {
      m_modules_by_type[mod->class_name()] = std::vector<std::string>();
    }
    m_modules_by_type[mod->class_name()].emplace_back(mod->UID());
  }
}

void
DAQModuleManager::init_modules(const std::vector<const dunedaq::confmodel::DaqModule*>& modules,
                               opmonlib::OpMonManager& opm)
{
  construct_modules(modules);

  for (const auto mod : modules) {
    auto mptr = m_module_map[mod->UID()];
    opm.register_node(mod->UID(), mptr);

    try {
      mptr->init(m_configuration_mgr);
    } catch (ers::Issue& ex) {
      throw DAQModuleInitFailed(ERS_HERE, mod->UID(), ex);
    }
  }
}

std::vector<ValidationReport>
DAQModuleManager::validate_action_plans(bool throw_on_fatal)
{
  std::vector<ValidationReport> reports;
  std::string app = m_configuration_mgr->get_app_name();

  for (auto& plan_pair : m_configuration_mgr->get_action_plans()) {
    auto cmd = plan_pair.first;
    TLOG_DEBUG(0) << app << ": Checking action plan " << cmd;
    std::map<std::string, std::set<std::string>> modules_with_cmd;
    for (const auto& [mod_type, module_list] : m_modules_by_type) {
      if (module_list.size() > 0 && m_module_map[module_list[0]]->has_command(cmd)) {
        modules_with_cmd[mod_type] = std::set<std::string>(module_list.begin(), module_list.end());
      }
    }

    for (auto& step : plan_pair.second->get_steps()) {
      auto byType = step->cast<confmodel::DaqModulesGroupByType>();
      auto byMod = step->cast<confmodel::DaqModulesGroupById>();
      if (byType != nullptr) {
        for (auto& mod_type : byType->get_modules()) {
          auto report = check_mod_has_cmd(cmd, mod_type, byType->get_optional(), "", throw_on_fatal);
          if (report)
            reports.push_back(report.value());
          modules_with_cmd.erase(mod_type);
        }
      } else if (byMod != nullptr) {
        for (auto& mod : byMod->get_modules()) {
          auto report = check_mod_has_cmd(cmd, mod->class_name(), byMod->get_optional(), mod->UID(), throw_on_fatal);
          if (report)
            reports.push_back(report.value());
          modules_with_cmd[mod->class_name()].erase(mod->UID());
        }
      } else {
        reports.emplace_back(
          ValidationReport::Severity::Fatal, app, "N/A", cmd, "Invalid subclass of DaqModulesGroup encountered!");
        if (throw_on_fatal)
          throw ActionPlanValidationFailed(
            ERS_HERE, reports.back().get_command(), reports.back().get_module(), reports.back().get_message());
        else
          ers::error(ActionPlanValidationFailed(
            ERS_HERE, reports.back().get_command(), reports.back().get_module(), reports.back().get_message()));
      }
    }

    for (const auto& [mod_type, module_list] : modules_with_cmd) {
      for (auto& mod : module_list) {
        reports.emplace_back(ValidationReport::Severity::Error,
                             app,
                             mod_type,
                             cmd,
                             "ActionPlan is defined, module has command, but module " + mod + " is not in any steps");
        ers::error(ActionPlanValidationFailed(
          ERS_HERE, reports.back().get_command(), reports.back().get_module(), reports.back().get_message()));
      }
    }
  }

  return reports;
}

void
DAQModuleManager::cleanup()
{
  get_iomanager()->reset();
  this->m_initialized = false;
}

DAQModule::CommandData_t
DAQModuleManager::get_command_data_for_module(const std::string& mod_name, const DAQModule::CommandData_t& cmd_data)
{
  auto cmd_obj = cmd_data.get<cmd::CmdObj>();
  const DAQModule::CommandData_t dummy{};

  if (!cmd_obj.modules.empty()) {
    for (const auto& addressed : cmd_obj.modules) {

      // First exception: empty = `all`
      if (addressed.match.empty()) {
        return static_cast<DAQModule::CommandData_t>(addressed.data);
      } else {
        // match module name with regex
        if (std::regex_match(mod_name, std::regex(addressed.match))) {
          return static_cast<DAQModule::CommandData_t>(addressed.data);
        }
      }
    }
  }
  // No matches
  return dummy;
}

bool
DAQModuleManager::execute_action(const std::string& module_name,
                                 const std::string& action,
                                 const DAQModule::CommandData_t& command_data)
{
  try {
    TLOG_DEBUG(2) << "Executing " << module_name << " -> " << action;
    m_module_map[module_name]->execute_command(action, command_data);
  } catch (ers::Issue& ex) {
    ers::error(ex);
    return false;
  }
  return true;
}

void
DAQModuleManager::execute_action_plan_step(std::string const& cmd,
                                           const confmodel::DaqModulesGroup* step,
                                           const DAQModule::CommandData_t& cmd_data,
                                           bool execution_mode_is_serial)
{
  std::string failed_mod_names("");
  std::unordered_map<std::string, std::future<bool>> futures;

  auto byType = step->cast<confmodel::DaqModulesGroupByType>();
  auto byMod = step->cast<confmodel::DaqModulesGroupById>();
  if (byType != nullptr) {
    for (auto& mod_class : byType->get_modules()) {
      auto modules = m_modules_by_type[mod_class];
      for (auto& mod_name : modules) {
        auto command_data = get_command_data_for_module(mod_name, cmd_data);
        TLOG_DEBUG(1) << "Executing action " << cmd << " on module " << mod_name << " (class " << mod_class << ")";
        futures[mod_name] =
          std::async(std::launch::async, &DAQModuleManager::execute_action, this, mod_name, cmd, command_data);
        if (execution_mode_is_serial)
          futures[mod_name].wait();
      }
    }
  } else if (byMod != nullptr) {
    for (auto& mod : byMod->get_modules()) {
      auto mod_name = mod->UID();
      auto command_data = get_command_data_for_module(mod_name, cmd_data);

      if (byMod->get_optional() && !m_module_map.count(mod_name)) {
        continue;
      }

      TLOG_DEBUG(1) << "Executing action " << cmd << " on module " << mod_name << " (class " << mod->class_name()
                    << ")";
      futures[mod_name] =
        std::async(std::launch::async, &DAQModuleManager::execute_action, this, mod_name, cmd, command_data);
      if (execution_mode_is_serial)
        futures[mod_name].wait();
    }
  } else {
    throw CommandDispatchingFailed(ERS_HERE, cmd, "Could not get DaqModulesGroup!");
  }

  for (auto& future : futures) {
    future.second.wait();
    auto ret = future.second.get();
    if (!ret) {
      failed_mod_names.append(future.first);
      failed_mod_names.append(", ");
    }
  }
  // Throw if any dispatching failed
  if (!failed_mod_names.empty()) {
    throw CommandDispatchingFailed(ERS_HERE, cmd, failed_mod_names);
  }
}

std::vector<std::string>
DAQModuleManager::get_modnames_by_cmdid(cmdlib::cmd::CmdId id)
{
  // Make a convenience array with module names that have the requested command
  std::vector<std::string> mod_names;
  for (const auto& [mod_name, mod_ptr] : m_module_map) {
    if (mod_ptr->has_command(id))
      mod_names.push_back(mod_name);
  }

  return mod_names;
}

void
DAQModuleManager::check_command_data(const std::string& id, const DAQModule::CommandData_t& cmd_data)
{
  // This method ensures that each module is only matched once per command.
  // If multiple matches are found, an ers::Issue is thrown
  // Disclaimenr for the occasional reader: this is the first implementation of the
  // multiple-matches detection logic. The author is painfully aware that it can be
  // vastly improved, in style if not in performance.

  auto cmd_obj = cmd_data.get<cmd::CmdObj>();
  const DAQModule::CommandData_t dummy{};

  // Make a convenience array with module names that have the requested command
  std::vector<std::string> cmd_mod_names = get_modnames_by_cmdid(id);

  // containers for error tracking
  std::map<std::string, std::vector<std::string>> mod_to_re;

  if (!cmd_obj.modules.empty()) {
    for (const auto& addressed : cmd_obj.modules) {
      if (!addressed.match.empty()) {
        // Find module names matching the regex
        for (const std::string& mod_name : cmd_mod_names) {
          // match module name with regex
          if (std::regex_match(mod_name, std::regex(addressed.match))) {
            mod_to_re[mod_name].push_back(addressed.match);
          }
        }
      }
    }

    // Select modules with multiple matches
    for (auto i = mod_to_re.begin(), last = mod_to_re.end(); i != last;) {
      if (i->second.size() == 1) {
        i = mod_to_re.erase(i);
      } else {
        ++i;
      }
    }

    // Catch cases
    if (mod_to_re.size() > 0) {
      std::string mod_names;
      for (const auto& [mod_name, matched_re] : mod_to_re) {
        mod_names += mod_name + ", ";
      }
      throw ConflictingCommandMatching(ERS_HERE, id, mod_names);
    }
  }
}

void
DAQModuleManager::execute(const std::string& cmd, const DAQModule::CommandData_t& cmd_data)
{

  TLOG_DEBUG(1) << "Command id:" << cmd;

  if (!m_initialized) {
    throw DAQModuleManagerNotInitialized(ERS_HERE, cmd);
  }

  check_command_data(cmd, cmd_data);

  auto action_plan = m_configuration_mgr->get_action_plan(cmd);
  if (action_plan == nullptr) {
    if (ACTION_PLANS_REQUIRED) {
      throw ActionPlanNotFound(ERS_HERE, cmd, "Throwing exception");
    } else if (ACTION_PLANS_REQUIRED_WARNING) {
      ers::warning(ActionPlanNotFound(ERS_HERE, cmd, "Returning without executing actions"));
      return;
    } else {
      // Emulate old behavior
      TLOG_DEBUG(1) << ActionPlanNotFound(ERS_HERE, cmd, "Executing action on all modules in parallel");
      std::string failed_mod_names("");
      std::unordered_map<std::string, std::future<bool>> futures;

      auto mods = get_modnames_by_cmdid(cmd);
      for (auto& mod : mods) {
        TLOG_DEBUG(1) << "Executing action " << cmd << " on module " << mod;
        auto command_data = get_command_data_for_module(mod, cmd_data);
        futures[mod] = std::async(std::launch::async, &DAQModuleManager::execute_action, this, mod, cmd, command_data);
      }

      for (auto& future : futures) {
        future.second.wait();
        auto ret = future.second.get();
        if (!ret) {
          failed_mod_names.append(future.first);
          failed_mod_names.append(", ");
        }
      }
      // Throw if any dispatching failed
      if (!failed_mod_names.empty()) {
        throw CommandDispatchingFailed(ERS_HERE, cmd, failed_mod_names);
      }
    }
  } else {
    auto execution_policy = action_plan->get_execution_policy();
    auto serial_execution = execution_policy == "modules-in-series";

    // We validated the action plans already
    for (auto& step : action_plan->get_steps()) {
      execute_action_plan_step(cmd, step, cmd_data, serial_execution);
    }
  }

  // Shutdown IOManager at scrap
  if (cmd == "scrap") {
    get_iomanager()->shutdown();
  }
}

} // namespace dunedaq::appfwk
