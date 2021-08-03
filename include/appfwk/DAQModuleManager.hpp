/**
 * @file DAQModuleManager.hpp Loads and distributes commands to DAQModules
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_DAQMODULEMANAGER_HPP_
#define APPFWK_INCLUDE_APPFWK_DAQMODULEMANAGER_HPP_

#include "appfwk/app/Structs.hpp"
#include "cmdlib/cmd/Structs.hpp"

#include "ers/Issue.hpp"
#include "nlohmann/json.hpp"
#include "opmonlib/InfoCollector.hpp"

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

  bool initialized() const { return m_initialized; }

  // Execute a properly structured command
  void execute(const dataobj_t& cmd_data);

  // Gather statistics from modules
  void gather_stats(opmonlib::InfoCollector& ic, int level);

protected:
  typedef std::map<std::string, std::shared_ptr<DAQModule>> DAQModuleMap_t; ///< DAQModules indexed by name

  void initialize(const dataobj_t& data);
  void init_queues(const app::QueueSpecs& qspecs);
  void init_modules(const app::ModSpecs& mspecs);

  void dispatch_one_match_only(cmdlib::cmd::CmdId id, const dataobj_t& data);
  void dispatch_after_merge(cmdlib::cmd::CmdId id, const dataobj_t& data);

private:
  std::vector<std::string> get_modnames_by_cmdid(cmdlib::cmd::CmdId id);

  bool m_initialized;

  DAQModuleMap_t m_module_map;
};

} // namespace appfwk
} // namespace dunedaq
#endif // APPFWK_INCLUDE_APPFWK_DAQMODULEMANAGER_HPP_
