
/**
 * @file Application.cpp Application implementataion
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "Application.hpp"

#include "appfwk/cmd/Nljs.hpp"
#include "appfwk/opmon/application.pb.h"

#include "confmodel/Application.hpp"
#include "confmodel/OpMonURI.hpp"
#include "confmodel/Session.hpp"
#include "logging/Logging.hpp"
#include "rcif/cmd/Nljs.hpp"

#include <string>
#include <unistd.h>
#include <utility>

namespace dunedaq::appfwk {

Application::Application(std::string app_name,
                         std::string session_name,
                         std::string cmdlibimpl,
                         std::string confimpl,
                         std::string configuration_id)
  : ConfigurationManagerOwner(confimpl, app_name, configuration_id)
  , OpMonManager(session_name, app_name, get_config_manager()->get_session()->get_opmon_uri()->get_URI(app_name))
  , NamedObject(app_name)
  , m_mod_mgr(session_name)
  , m_state("NONE")
  , m_busy(false)
  , m_error(false)
  , m_initialized(false)
{
  m_runinfo.set_running(false);
  m_runinfo.set_run_number(0);
  m_runinfo.set_run_time(0);

  m_cmd_fac = cmdlib::make_command_facility(
    cmdlibimpl, session_name, get_config_manager()->get_session()->get_connectivity_service());

  set_opmon_conf(get_config_manager()->get_application()->get_opmon_conf());

  TLOG() << "confimpl=<" << confimpl << ">\n";
}

void
Application::init()
{
  m_cmd_fac->set_commanded(*this, get_name());
  m_mod_mgr.initialize(get_config_manager(), *this);
  set_state("INITIAL");
  m_initialized = true;
}

void
Application::run(std::atomic<bool>& end_marker)
{
  if (!m_initialized) {
    throw ApplicationNotInitialized(ERS_HERE, get_name());
  }

  start_monitoring();
  m_cmd_fac->run(end_marker);

  m_mod_mgr.cleanup();

  stop_monitoring();
}

void
Application::execute(const dataobj_t& cmd_data)
{
  auto rc_cmd = cmd_data.get<rcif::cmd::RCCommand>();
  std::string cmdname = rc_cmd.id;
  if (!check_state_for_cmd(cmd_data)) {
    throw InvalidStateForCommand(ERS_HERE, cmdname, get_state(), m_error.load(), m_busy.load());
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
  } else if (cmdname == "stop") {
    m_run_start_time = std::chrono::steady_clock::time_point();
    m_runinfo.set_running(false);
    m_runinfo.set_run_number(0);
    m_runinfo.set_run_time(0);
  } else if (cmdname == "conf" || cmdname == "reload") { // Does not
                                                         // belong
                                                         // here, just
                                                         // hacked
                                                         // in for testing!!!
    std::cout << "rc_cmd.data=" << rc_cmd.data << "\n";

    std::string confspec{""};
    if (rc_cmd.data.contains("confspec")) {
      confspec = rc_cmd.data["confspec"];
    }
    get_config_manager()->reload(confspec);
    m_mod_mgr.reload(*dynamic_cast<OpMonManager*>(this));
  }

  try {
    m_mod_mgr.execute(cmdname, static_cast<DAQModule::CommandData_t>(rc_cmd.data));
    m_busy.store(false);
    if (rc_cmd.exit_state != "ANY")
      set_state(rc_cmd.exit_state);
  } catch (ers::Issue& ex) {
    m_busy.store(false);
    m_error.store(true);
    throw;
  }

  publish_app_info();
}

void
Application::generate_opmon_data()
{
  publish_app_info();
}

bool
Application::check_state_for_cmd(const dataobj_t& cmd_data) const
{
  if (m_busy.load() || m_error.load())
    return false;

  std::string entry_state = cmd_data.get<rcif::cmd::RCCommand>().entry_state;
  if (entry_state == "ANY" || get_state() == entry_state)
    return true;

  return false;
}

void
Application::publish_app_info() {
  
  opmon::AppInfo ai;
  ai.set_state(get_state());
  ai.set_busy(m_busy.load());
  ai.set_error(m_error.load());
  
  char hostname[256]; // NOLINT
  auto res = gethostname(hostname, 256);
  if (res < 0)
    ai.set_host("Unknown");
  else
    ai.set_host(std::string(hostname));

  publish(std::move(ai), {}, opmonlib::to_level(opmonlib::EntryOpMonLevel::kTopPriority));

  if (m_run_start_time.time_since_epoch().count() != 0) {
    auto now = std::chrono::steady_clock::now();
    m_runinfo.set_run_time(std::chrono::duration_cast<std::chrono::seconds>(now - m_run_start_time).count());
  }

  publish(decltype(m_runinfo)(m_runinfo));

}

  
} // namespace dunedaq::appfwk
