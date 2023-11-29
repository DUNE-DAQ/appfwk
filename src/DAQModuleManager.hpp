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

#include "coredal/DaqModule.hpp"
#include "oksdbinterfaces/Configuration.hpp"
#include "appfwk/ConfigurationManager.hpp"
#include "appfwk/ModuleConfiguration.hpp"

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
// Re-enable coverage collection LCOV_EXCL_STOP

namespace appfwk {

class DAQModule;

class DAQModuleManager
{
public:
  using dataobj_t = nlohmann::json;

  DAQModuleManager();
  
  void initialize();
  bool initialized() const { return m_initialized; }
  void cleanup();

  // Execute a properly structured command
  void execute(const std::string& state, const std::string& cmd, const dataobj_t& cmd_data);

  // Gather statistics from modules
  void gather_stats(opmonlib::InfoCollector& ic, int level);

private:
  typedef std::map<std::string, std::shared_ptr<DAQModule>> DAQModuleMap_t; ///< DAQModules indexed by name

  void init_modules(const std::vector<const dunedaq::coredal::DaqModule*>& modules);
  void dispatch_one_match_only(cmdlib::cmd::CmdId id, const std::string& state, const dataobj_t& data);
  void dispatch_after_merge(cmdlib::cmd::CmdId id, const std::string& state, const dataobj_t& data);

  std::vector<std::string> get_modnames_by_cmdid(cmdlib::cmd::CmdId id, const std::string& state);

  bool m_initialized;

  DAQModuleMap_t m_module_map;
};

} // namespace appfwk
} // namespace dunedaq

#include "detail/DAQModuleManager.hxx"

#endif // APPFWK_INCLUDE_APPFWK_DAQMODULEMANAGER_HPP_
