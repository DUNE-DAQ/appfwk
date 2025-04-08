/**
 * @file Application.hpp Loads and distributes commands to DAQModules
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_SRC_APPLICATION_HPP_
#define APPFWK_SRC_APPLICATION_HPP_

// appfwk Includes
#include "ConfigurationManagerOwner.hpp"
#include "DAQModuleManager.hpp"
#include "appfwk/cmd/Structs.hpp"

// DUNE-DAQ includes
#include "cmdlib/CommandFacility.hpp"
#include "cmdlib/CommandedObject.hpp"
#include "logging/Logging.hpp" // NOTE: if ISSUES ARE DECLARED BEFORE include logging/Logging.hpp, TLOG_DEBUG<<issue wont work.
#include "opmonlib/OpMonManager.hpp"
#include "rcif/opmon/run_info.pb.h"
#include "utilities/NamedObject.hpp"

// External libraries
#include "nlohmann/json.hpp"

// C++ includes
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>

namespace dunedaq {

// Disable coverage collection LCOV_EXCL_START

ERS_DECLARE_ISSUE(appfwk,
                  ApplicationNotInitialized,
                  "Application " << name << " has not been initialized yet.",
                  ((std::string)name))

ERS_DECLARE_ISSUE(appfwk,
                  InvalidStateForCommand,
                  "Command " << cmdid << " not allowed at this time. state: " << state << ", error: " << err
                             << ", busy: " << busy,
                  ((std::string)cmdid)((std::string)state)((bool)err)((bool)busy)) // NOLINT

ERS_DECLARE_ISSUE(appfwk,
                  ApplicationFailure,
                  "Application " << application << " in session " << session << " failed",
                  ((std::string)session)((std::string)application))

// Re-enable coverage collection LCOV_EXCL_STOP

namespace appfwk {

class Application
  : public ConfigurationManagerOwner
  , public cmdlib::CommandedObject
  , public opmonlib::OpMonManager
  , public utilities::NamedObject
{
public:
  using dataobj_t = nlohmann::json;

  Application(std::string app_name,
              std::string session_name,
              std::string cmdlibimpl,
              std::string confimpl,
              std::string configuration_id);

  void init();

  void run(std::atomic<bool>& end_marker);

  void execute(const dataobj_t& cmd_data) override;

  bool check_state_for_cmd(const dataobj_t& cmd_data) const;

  void generate_opmon_data() override;

  void set_state(std::string s)
  {
    const std::lock_guard<std::mutex> lock(m_mutex);
    m_state = s;
  }
  std::string get_state() const
  {
    const std::lock_guard<std::mutex> lock(m_mutex);
    return m_state;
  }

private:
  DAQModuleManager m_mod_mgr;
  mutable std::mutex m_mutex;
  std::string m_state;
  std::atomic<bool> m_busy;
  std::atomic<bool> m_error;
  bool m_initialized;
  std::chrono::time_point<std::chrono::steady_clock> m_run_start_time;
  dunedaq::rcif::opmon::RunInfo m_runinfo;
  std::shared_ptr<cmdlib::CommandFacility> m_cmd_fac;
};

} // namespace appfwk
} // namespace dunedaq

#endif // APPFWK_SRC_APPLICATION_HPP_
