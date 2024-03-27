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
#include "appfwk/app/Nljs.hpp"
#include "appfwk/cmd/Nljs.hpp"

#include "appfwk/DAQModule.hpp"

#include "coredal/Action.hpp"
#include "coredal/ActionStep.hpp"
#include "coredal/Session.hpp"

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
DAQModuleManager::initialize(std::shared_ptr<ConfigurationManager> cfgMgr)
{
  auto csInterval = cfgMgr->session()->get_connectivity_service_interval_ms();
  m_module_configuration = std::make_shared<ModuleConfiguration>(cfgMgr);
  get_iomanager()->configure(m_module_configuration->queues(),
                             m_module_configuration->networkconnections(),
                             true,
                             std::chrono::milliseconds(csInterval));
  init_modules(m_module_configuration->modules());

  for (auto& plan_pair : m_module_configuration->action_plans()) {
    auto cmd = plan_pair.first;

    for (auto& step : plan_pair.second->get_steps()) {
      for (auto& action : step->get_actions()) {
        if (action->get_module() == "")
          continue;
        if (!m_module_map.count(action->get_module())) {
          throw ActionPlanValidationFailed(ERS_HERE, cmd, action->get_module(), "Module does not exist");
        }
        if (!m_module_map[action->get_module()]->has_command(action->get_method_name())) {
          throw ActionPlanValidationFailed(
            ERS_HERE, cmd, action->get_module(), "Module does not have method " + action->get_method_name());
        }
      }
    }
  }

  this->m_initialized = true;
}

void
DAQModuleManager::init_modules(const std::vector<const dunedaq::coredal::DaqModule*>& modules)
{
  for (const auto mod : modules) {
    TLOG_DEBUG(0) << "construct: " << mod->class_name() << " : " << mod->UID();
    auto mptr = make_module(mod->class_name(), mod->UID());
    m_module_map.emplace(mod->UID(), mptr);
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
                                           const coredal::ActionStep* step,
                                           const dataobj_t& cmd_data)
{
  std::string failed_mod_names("");
  std::unordered_map<std::string, std::future<bool>> futures;

  for (auto& action : step->get_actions()) {
    auto mod_name = action->get_module();

    if (mod_name == "" || mod_name == "*") {
      auto mods = get_modnames_by_cmdid(action->get_method_name());
      for (auto& mod : mods) {
        TLOG_DEBUG(1) << "Executing action " << action->get_method_name() << " on module " << mod;
        auto data_obj = get_dataobj_for_module(mod, cmd_data);
        futures[mod] = std::async(
          std::launch::async, &DAQModuleManager::execute_action, this, mod, action->get_method_name(), data_obj);
      }
    } else {
      auto data_obj = get_dataobj_for_module(action->get_module(), cmd_data);
      TLOG_DEBUG(1) << "Executing action " << action->get_method_name() << " on module " << action->get_module();
      futures[action->get_module()] = std::async(std::launch::async,
                                                 &DAQModuleManager::execute_action,
                                                 this,
                                                 action->get_module(),
                                                 action->get_method_name(),
                                                 data_obj);
    }
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
    ers::info(
      ActionPlanNotFound(ERS_HERE, cmd, "Executing action on all modules in parallel"));
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
    // We validated the action plans already
    for (auto& step : action_plan->get_steps()) {
      execute_action_plan_step(cmd, step, cmd_data);
    }
  }
}

void
DAQModuleManager::gather_stats(opmonlib::InfoCollector& ci, int level)
{

  iomanager::QueueRegistry::get().gather_stats(ci, level);
  iomanager::NetworkManager::get().gather_stats(ci, level);

  for (const auto& [mod_name, mod_ptr] : m_module_map) {
    try {
      opmonlib::InfoCollector tmp_ci;
      mod_ptr->get_info(tmp_ci, level);
      if (!tmp_ci.is_empty()) {
        ci.add(mod_name, tmp_ci);
      }
    } catch (ers::Issue& i) {
      ers::warning(FailedInfoGathering(ERS_HERE, mod_name, i));
    } catch (std::exception& ex) {
      ers::warning(ExceptionWhileInfoGathering(ERS_HERE, mod_name, ex.what()));
    } catch (...) {
      ers::warning(FailedInfoGathering(ERS_HERE, mod_name));
    }
  }
}

} // namespace appfwk
} // namespace dunedaq
