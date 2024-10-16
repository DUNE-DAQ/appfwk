/**
 * @file DAQModuleManager.cpp DAQModuleManager implementataion
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "DAQModuleManager.hpp"

#include "cmdlib/cmd/Nljs.hpp"

#include "appfwk/Issues.hpp"
#include "appfwk/cmd/Nljs.hpp"

#include "appfwk/DAQModule.hpp"

#include "confmodel/DaqModulesGroup.hpp"
#include "confmodel/DaqModulesGroupById.hpp"
#include "confmodel/DaqModulesGroupByType.hpp"
#include "confmodel/Session.hpp"

#include "iomanager/IOManager.hpp"

#include "logging/Logging.hpp"

#include <future>
#include <map>
#include <regex>
#include <string>
#include <utility>
#include <vector>

namespace dunedaq {
namespace appfwk {

DAQModuleManager::DAQModuleManager()
  : m_initialized(false)
{
}

void
DAQModuleManager::initialize(std::shared_ptr<ConfigurationManager> cfgMgr, opmonlib::OpMonManager& opm)
{
  m_module_configuration = std::make_shared<ModuleConfiguration>(cfgMgr);
  get_iomanager()->configure(cfgMgr->session()->UID(),
                             m_module_configuration->queues(),
                             m_module_configuration->networkconnections(),
                             m_module_configuration->connectivity_service(),
                             opm);
  init_modules(m_module_configuration->modules(), opm);

  for (auto& plan_pair : m_module_configuration->action_plans()) {
    auto cmd = plan_pair.first;

    for (auto& step : plan_pair.second->get_steps()) {
      auto byType = step->cast<confmodel::DaqModulesGroupByType>();
      auto byMod = step->cast<confmodel::DaqModulesGroupById>();
      if (byType != nullptr) {
        for (auto& mod_type : byType->get_modules()) {
          check_mod_has_cmd(cmd, mod_type);
        }
      } else if (byMod != nullptr) {
        for (auto& mod : byMod->get_modules()) {
          check_mod_has_cmd(cmd, mod->class_name(), mod->UID());
        }
      } else {
        throw ActionPlanValidationFailed(ERS_HERE, cmd, "", "Invalid subclass of DaqModulesGroup encountered!");
      }
    }
  }
  this->m_initialized = true;
}

void
DAQModuleManager::check_mod_has_cmd(const std::string& cmd, const std::string& mod_class, const std::string& mod_id)
{

  if (!m_modules_by_type.count(mod_class) || m_modules_by_type[mod_class].size() == 0) {
    auto issue = ActionPlanValidationFailed(ERS_HERE, cmd, mod_class, "Module does not exist");
    if (mod_id == "") {
      ers::info(issue);
      return;
    } else {
      throw issue;
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
    if (!match) {
      throw ActionPlanValidationFailed(ERS_HERE, cmd, mod_class, "No module with id " + mod_id + " found.");
    }
  }

  if (!module_test->has_command(cmd)) {
    throw ActionPlanValidationFailed(ERS_HERE, cmd, mod_class, "Module does not have method " + cmd);
  }
}

void
DAQModuleManager::init_modules(const std::vector<const dunedaq::confmodel::DaqModule*>& modules,
                               opmonlib::OpMonManager& opm)
{
  for (const auto mod : modules) {
    TLOG_DEBUG(0) << "construct: " << mod->class_name() << " : " << mod->UID();
    auto mptr = make_module(mod->class_name(), mod->UID());
    m_module_map.emplace(mod->UID(), mptr);

    if (!m_modules_by_type.count(mod->class_name())) {
      m_modules_by_type[mod->class_name()] = std::vector<std::string>();
    }
    m_modules_by_type[mod->class_name()].emplace_back(mod->UID());

    opm.register_node(mod->UID(), mptr);
    mptr->init(m_module_configuration);
  }
}

void
DAQModuleManager::cleanup()
{
  get_iomanager()->reset();
  this->m_initialized = false;
}

DAQModuleManager::dataobj_t
DAQModuleManager::get_dataobj_for_module(const std::string& mod_name, const dataobj_t& cmd_data)
{
  auto cmd_obj = cmd_data.get<cmd::CmdObj>();
  const dataobj_t dummy{};

  if (!cmd_obj.modules.empty()) {
    for (const auto& addressed : cmd_obj.modules) {

      // First exception: empty = `all`
      if (addressed.match.empty()) {
        return addressed.data;
      } else {
        // match module name with regex
        if (std::regex_match(mod_name, std::regex(addressed.match))) {
          return addressed.data;
        }
      }
    }
  }
  // No matches
  return dummy;
}

bool
DAQModuleManager::execute_action(const std::string& module_name, const std::string& action, const dataobj_t& data_obj)
{
  try {
    TLOG_DEBUG(2) << "Executing " << module_name << " -> " << action;
    m_module_map[module_name]->execute_command(action, data_obj);
  } catch (ers::Issue& ex) {
    ers::error(ex);
    return false;
  }
  return true;
}

void
DAQModuleManager::execute_action_plan_step(std::string const& cmd,
                                           const confmodel::DaqModulesGroup* step,
                                           const dataobj_t& cmd_data,
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
        auto data_obj = get_dataobj_for_module(mod_name, cmd_data);
        TLOG_DEBUG(1) << "Executing action " << cmd << " on module " << mod_name << " (class " << mod_class << ")";
        futures[mod_name] =
          std::async(std::launch::async, &DAQModuleManager::execute_action, this, mod_name, cmd, data_obj);
        if (execution_mode_is_serial)
          futures[mod_name].wait();
      }
    }
  } else if (byMod != nullptr) {
    for (auto& mod : byMod->get_modules()) {
      auto mod_name = mod->UID();
      auto data_obj = get_dataobj_for_module(mod_name, cmd_data);
      TLOG_DEBUG(1) << "Executing action " << cmd << " on module " << mod_name << " (class " << mod->class_name()
                    << ")";
      futures[mod_name] =
        std::async(std::launch::async, &DAQModuleManager::execute_action, this, mod_name, cmd, data_obj);
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
DAQModuleManager::check_cmd_data(const std::string& id, const dataobj_t& cmd_data)
{
  // This method ensures that each module is only matched once per command.
  // If multiple matches are found, an ers::Issue is thrown
  // Disclaimenr for the occasional reader: this is the first implementation of the
  // multiple-matches detection logic. The author is painfully aware that it can be
  // vastly improved, in style if not in performance.

  auto cmd_obj = cmd_data.get<cmd::CmdObj>();
  const dataobj_t dummy{};

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
DAQModuleManager::execute(const std::string& cmd, const dataobj_t& cmd_data)
{

  TLOG_DEBUG(1) << "Command id:" << cmd;

  if (!m_initialized) {
    throw DAQModuleManagerNotInitialized(ERS_HERE, cmd);
  }

  check_cmd_data(cmd, cmd_data);

  auto action_plan = m_module_configuration->action_plan(cmd);
  if (action_plan == nullptr) {
#if 0
    throw ActionPlanNotFound(ERS_HERE, cmd, "Throwing exception");
#elif 0
    ers::warning(ActionPlanNotFound(ERS_HERE, cmd, "Returning without executing actions"));
    return;
#else
    // Emulate old behavior
    TLOG_DEBUG(1) << ActionPlanNotFound(ERS_HERE, cmd, "Executing action on all modules in parallel");
    std::string failed_mod_names("");
    std::unordered_map<std::string, std::future<bool>> futures;

    auto mods = get_modnames_by_cmdid(cmd);
    for (auto& mod : mods) {
      TLOG_DEBUG(1) << "Executing action " << cmd << " on module " << mod;
      auto data_obj = get_dataobj_for_module(mod, cmd_data);
      futures[mod] = std::async(std::launch::async, &DAQModuleManager::execute_action, this, mod, cmd, data_obj);
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
#endif
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

} // namespace appfwk
} // namespace dunedaq
