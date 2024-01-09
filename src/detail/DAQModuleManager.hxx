/**
 * @file DAQModuleManager.cpp DAQModuleManager implementataion
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "cmdlib/cmd/Nljs.hpp"

#include "appfwk/Issues.hpp"
#include "appfwk/app/Nljs.hpp"
#include "appfwk/cmd/Nljs.hpp"

#include "appfwk/DAQModule.hpp"

#include "coredal/Session.hpp"

#include "iomanager/IOManager.hpp"

#include "logging/Logging.hpp"

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

void
DAQModuleManager::dispatch_after_merge(cmdlib::cmd::CmdId id, const std::string& state, const dataobj_t& data)
{
  // The command dispatching: commands and parameters are distributed to all modules that
  // have registered a method corresponding to the command. If no parameters are found, an
  // empty dataobj_t is passed.
  std::string bad_mod_names("");
  auto cmd_obj = data.get<cmd::CmdObj>();
  for (const auto& [mod_name, mod_ptr] : m_module_map) {
    if (mod_ptr->has_command(id, state)) {
      dataobj_t params;
      for (const auto& addressed : cmd_obj.modules) {
        if (addressed.match.empty() || std::regex_match(mod_name.c_str(), std::regex(addressed.match.c_str()))) {
          for (nlohmann::json::const_iterator it = addressed.data.begin(); it != addressed.data.end(); ++it) {
            params[it.key()] = it.value();
          }
        }
      }
      TLOG_DEBUG(2) << "Dispatch \"" << id << "\" to \"" << mod_ptr->get_name() << "\":\n" << params.dump(4);
      try {
        mod_ptr->execute_command(id, state, params);
      } catch (ers::Issue& ex) {
        ers::error(ex);
        bad_mod_names.append(mod_name);
        bad_mod_names.append(", ");
      }
    }
  }
  if (!bad_mod_names.empty()) {
    throw CommandDispatchingFailed(ERS_HERE, id, bad_mod_names);
  }
}

std::vector<std::string>
DAQModuleManager::get_modnames_by_cmdid(cmdlib::cmd::CmdId id, const std::string& state)
{
  // Make a convenience array with module names that have the requested command
  std::vector<std::string> mod_names;
  for (const auto& [mod_name, mod_ptr] : m_module_map) {
    if (mod_ptr->has_command(id, state))
      mod_names.push_back(mod_name);
  }

  return mod_names;
}

void
DAQModuleManager::dispatch_one_match_only(cmdlib::cmd::CmdId id, const std::string& state, const dataobj_t& data)
{
  // This method ensures that each module is only matched once per command.
  // If multiple matches are found, an ers::Issue is thrown
  // Disclaimenr for the occasional reader: this is the first implementation of the
  // multiple-matches detection logic. The author is painfully aware that it can be
  // vastly improved, in style if not in performance.

  auto cmd_obj = data.get<cmd::CmdObj>();
  const dataobj_t dummy{};

  // Make a convenience array with module names that have the requested command
  std::vector<std::string> cmd_mod_names = get_modnames_by_cmdid(id, state);

  // containers for error tracking
  std::vector<std::string> unmatched_addr;
  std::map<std::string, std::vector<std::string>> mod_to_re;

  std::vector<std::pair<std::vector<std::string>, const dataobj_t*>> mod_seq;

  if (!cmd_obj.modules.empty()) {
    for (const auto& addressed : cmd_obj.modules) {

      // Module names matching the 'match' regex
      std::vector<std::string> matches;

      // First exception: empty = `all`
      if (addressed.match.empty()) {
        matches = cmd_mod_names;
      } else {
        // Find module names matching the regex
        for (const std::string& mod_name : cmd_mod_names) {
          // match module name with regex
          if (std::regex_match(mod_name, std::regex(addressed.match))) {
            matches.push_back(mod_name);
            mod_to_re[mod_name].push_back(addressed.match);
          }
        }

        // Keep track of unmatched expressions
        if (matches.empty()) {
          unmatched_addr.push_back(addressed.match);
          continue;
        }
      }
      mod_seq.emplace_back(matches, &addressed.data);
    }

    if (!unmatched_addr.empty()) {
      // say something!
      // or not?
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

  } else {
    mod_seq.emplace_back(cmd_mod_names, &dummy);
  }

  std::string failed_mod_names("");

  // All sorted, execute!
  for (auto& [mod_names, data_ptr] : mod_seq) {
    for (auto& mod_name : mod_names) {
      try {
        TLOG_DEBUG(2) << "Executing " << id << " -> " << mod_name;
        m_module_map[mod_name]->execute_command(id, state, *data_ptr);
      } catch (ers::Issue& ex) {
        ers::error(ex);
        failed_mod_names.append(mod_name);
        failed_mod_names.append(", ");
      }
    }
  }

  // Throw if any dispatching failed
  if (!failed_mod_names.empty()) {
    throw CommandDispatchingFailed(ERS_HERE, id, failed_mod_names);
  }
}

void
DAQModuleManager::execute(const std::string& state, const std::string& cmd, const dataobj_t& cmd_data)
{

  TLOG_DEBUG(1) << "Command id:" << cmd;

  if (!m_initialized) {
    throw DAQModuleManagerNotInitialized(ERS_HERE, cmd);
  }

  dispatch_one_match_only(cmd, state, cmd_data);

  // dispatch(cmd.id, cmd.data);
}

void
DAQModuleManager::gather_stats(opmonlib::InfoCollector& ci, int level)
{

  iomanager::QueueRegistry::get().gather_stats(ci, level);
  iomanager::NetworkManager::get().gather_stats(ci, level);

  for (const auto& [mod_name, mod_ptr] : m_module_map) {
    opmonlib::InfoCollector tmp_ci;
    mod_ptr->get_info(tmp_ci, level);
    if (!tmp_ci.is_empty()) {
      ci.add(mod_name, tmp_ci);
    }
  }
}

} // namespace appfwk
} // namespace dunedaq
