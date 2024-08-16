/**
 * @file Application.cpp Application implementataion
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "Application.hpp"

#include "appfwk/Issues.hpp"
#include "appfwk/opmon/application.pb.h"
#include "appfwk/cmd/Nljs.hpp"
#include "rcif/cmd/Nljs.hpp"

#include "logging/Logging.hpp"

#include <string>
#include <unistd.h>

namespace dunedaq {
namespace appfwk {

Application::Application(std::string appname,
                         std::string session,
                         std::string cmdlibimpl,
                         std::string opmonlibimpl,
                         std::string confimpl)
  : OpMonManager(session, appname, opmonlibimpl)
  , NamedObject(appname)
  , m_state("NONE")
  , m_busy(false)
  , m_error(false)
  , m_initialized(false)
  , m_config_mgr(std::make_shared<ConfigurationManager>(confimpl, appname, session))
{
  m_runinfo.set_running(false);
  m_runinfo.set_run_number(0);
  m_runinfo.set_run_time(0);

  m_cmd_fac = cmdlib::make_command_facility(cmdlibimpl);

  TLOG() << "confimpl=<" << confimpl << ">\n";
}

void
Application::init()
{
  m_cmd_fac->set_commanded(*this, get_name());
  m_mod_mgr.initialize(m_config_mgr, *this);
  set_state("INITIAL");
  m_initialized = true;
}

void
Application::run(std::atomic<bool>& end_marker)
{
  if (!m_initialized) {
    throw ApplicationNotInitialized(ERS_HERE, get_name());
  }

  setenv("DUNEDAQ_OPMON_INTERVAL", "10", 0);
  setenv("DUNEDAQ_OPMON_LEVEL", "1", 0);

  std::stringstream s1(getenv("DUNEDAQ_OPMON_INTERVAL"));
  std::stringstream s2(getenv("DUNEDAQ_OPMON_LEVEL"));
  uint32_t interval = 0; // NOLINT(build/unsigned)
  opmonlib::OpMonLevel level  = 0; 
  s1 >> interval;
  s2 >> level;

  // MR: we need to set the proper level here
  set_opmon_level(dunedaq::opmonlib::to_level(dunedaq::opmonlib::SystemOpMonLevel::kAll));

  start_monitoring(std::chrono::seconds(interval));
  m_cmd_fac->run(end_marker);

  m_mod_mgr.cleanup();
}

void
Application::execute(const dataobj_t& cmd_data)
{
  auto rc_cmd = cmd_data.get<rcif::cmd::RCCommand>();
  std::string cmdname = rc_cmd.id;
  if (!is_cmd_valid(cmd_data)) {
    throw InvalidCommand(ERS_HERE, cmdname, get_state(), m_error.load(), m_busy.load());
  }

  m_busy.store(true);

  if (cmdname == "start") {
    auto cmd_obj = rc_cmd.data.get<cmd::CmdObj>();
    
    for (const auto& addressed : cmd_obj.modules) {
      dataobj_t startpars = addressed.data;
      auto rc_startpars = startpars.get<rcif::cmd::StartParams>();
      m_runinfo.set_run_number(rc_startpars.run);
      break;
    }
    
    m_run_start_time = std::chrono::steady_clock::now();
    m_runinfo.set_running(true);
    m_runinfo.set_run_time(0);
  }
  else if (cmdname == "stop") {
    m_run_start_time = std::chrono::steady_clock::time_point();
    m_runinfo.set_running(false);
    m_runinfo.set_run_number(0);
    m_runinfo.set_run_time(0);
  }
  
  try {
    m_mod_mgr.execute(cmdname, rc_cmd.data);
    m_busy.store(false);
    if (rc_cmd.exit_state != "ANY")
      set_state(rc_cmd.exit_state);
  } catch (ers::Issue& ex) {
    m_busy.store(false);
    m_error.store(true);
    throw;
  }
}

void
Application::generate_opmon_data()
{
  opmon::AppInfo ai;
  ai.set_state(get_state());
  ai.set_busy(m_busy.load());
  ai.set_error(m_error.load());

  char hostname[256];
  auto res = gethostname(hostname, 256);
  if (res < 0)
    ai.set_host("Unknown");
  else
    ai.set_host (std::string(hostname));

  publish(std::move(ai), {}, opmonlib::to_level(opmonlib::EntryOpMonLevel::kTopPriority));
  
  if ( m_run_start_time.time_since_epoch().count() == 0 ) {
    auto now = std::chrono::steady_clock::now();
    m_runinfo.set_run_time(std::chrono::duration_cast<std::chrono::seconds>(now - m_run_start_time).count() );
  }

  publish( decltype(m_runinfo)(m_runinfo) );
}

bool
Application::is_cmd_valid(const dataobj_t& cmd_data)
{
  if (m_busy.load() || m_error.load())
    return false;

  std::string state = get_state();
  std::string entry_state = cmd_data.get<rcif::cmd::RCCommand>().entry_state;
  if (entry_state == "ANY" || state == entry_state)
    return true;

  return false;
}

} // namespace appfwk
} // namespace dunedaq
