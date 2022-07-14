/**
 * @file Application.hpp Loads and distributes commands to DAQModules
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_APPLICATION_HPP_
#define APPFWK_INCLUDE_APPFWK_APPLICATION_HPP_


#include "appfwk/cmd/Structs.hpp"
#include "rcif/runinfo/InfoStructs.hpp"
#include "utilities/NamedObject.hpp"

#include "cmdlib/CommandFacility.hpp"
#include "cmdlib/CommandedObject.hpp"

#include "DAQModuleManager.hpp"
#include "appfwk/ConfFacility.hpp"

#include "opmonlib/InfoManager.hpp"
#include "opmonlib/InfoProvider.hpp"
 
#include "ers/Issue.hpp"
#include "nlohmann/json.hpp"

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>

namespace dunedaq {

// Disable coverage collection LCOV_EXCL_START
/**
 * @brief A generic Application ERS Issue
 */
ERS_DECLARE_ISSUE(appfwk,                                                     ///< Namespace
                  ApplicationNotInitialized,                                  ///< Issue class name
                  "Application " << name << " has not been initialized yet.", ///< Message
                  ((std::string)name)                                         ///< Message parameters
)

ERS_DECLARE_ISSUE(appfwk,         ///< Namespace
                  InvalidCommand, ///< Issue class name
                  "Command " << cmdid << " not allowed. state: " << state << ", error: " << err
                             << ", busy: " << busy, ///< Message
                  ((std::string)cmdid)              ///< Message parameters
                  ((std::string)state)              ///< Message parameters
                  ((bool)err)                       ///< Message parameters // NOLINT
                  ((bool)busy)                      ///< Message parameters // NOLINT
)

// Re-enable coverage collection LCOV_EXCL_STOP
namespace appfwk {

class Application
  : public cmdlib::CommandedObject
  , public opmonlib::InfoProvider
  , public utilities::NamedObject
{
public:
  using dataobj_t = nlohmann::json;

  Application(std::string app_name, std::string partition_name, std::string cmdlibimpl, std::string opmonlibimpl, std::string confimpl);

  // Initialize the application services
  void init();

  // Start the main run loop
  void run(std::atomic<bool>& end_marker);

  // Execute a properly structured command
  void execute(const dataobj_t& cmd_data);

  // Gether the opmon information

  void gather_stats(opmonlib::InfoCollector& ic, int level);

  // Check whether the command can be accepted
  bool is_cmd_valid(const dataobj_t& cmd_data);

  // State synch getter & setter

  void set_state(std::string s)
  {
    const std::lock_guard<std::mutex> lock(m_mutex);
    m_state = s;
  }
  std::string get_state()
  {
    const std::lock_guard<std::mutex> lock(m_mutex);
    return m_state;
  }

private:
  std::mutex m_mutex;
  std::string m_partition;
  opmonlib::InfoManager m_info_mgr;
  std::string m_state;
  std::atomic<bool> m_busy;
  std::atomic<bool> m_error;
  bool m_initialized;
  std::chrono::time_point<std::chrono::steady_clock> m_run_start_time;
  dunedaq::rcif::runinfo::Info m_runinfo;
  std::string m_fully_qualified_name;
  DAQModuleManager m_mod_mgr;
  std::shared_ptr<cmdlib::CommandFacility> m_cmd_fac;
  std::shared_ptr<ConfFacility> m_conf_fac;
};

} // namespace appfwk
} // namespace dunedaq

#include "detail/Application.hxx"

#endif // APPFWK_INCLUDE_APPFWK_APPLICATION_HPP_
