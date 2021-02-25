/**
 * @file Application.cpp Application implementataion
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/Application.hpp"

#include "appfwk/Issues.hpp"
#include "appfwk/cmd/Nljs.hpp"
#include "appfwk/appinfo/Nljs.hpp"

#include "ers/ers.hpp"

namespace dunedaq {
namespace appfwk {

Application::Application(std::string appname, std::string partition, std::string cmdlibimpl, std::string opmonlibimpl)
  : NamedObject(appname), m_partition(partition), m_info_mgr(opmonlibimpl), m_state("NONE"), m_busy(false), m_error(false),
    m_initialized(false) 
{
   m_cmd_fac = cmdlib::makeCommandFacility(cmdlibimpl);
}

void
Application::init()
{
  m_cmd_fac->set_commanded(*this);
  m_info_mgr.set_provider(*this);
  m_initialized = true;
}

void
Application::run(std::atomic<bool>& end_marker)
{
  if (!m_initialized) {
    throw ApplicationNotInitialized(ERS_HERE, get_name());
  }
  m_info_mgr.start(5, 3);
  m_cmd_fac->run(end_marker);
  m_info_mgr.stop();
}

void
Application::execute(const dataobj_t& cmd_data)
{

  std::string cmdname = cmd_data.get<cmdlib::cmd::Command>().id;
  if(!is_cmd_valid(cmd_data)) {
    throw InvalidCommand(ERS_HERE, cmdname, get_state(), m_error.load(), m_busy.load());
  }

  m_busy.store(true);

  try {
    m_mod_mgr.execute(cmd_data);
    m_busy.store(false);
    if (cmdname == "init" || cmdname == "scrap") {
       set_state("INITIAL") ; 
    }
    else if (cmdname == "conf" || cmdname == "stop") {
       set_state("CONFIGURED") ;
     
    }
    else if (cmdname == "start" || cmdname == "resume") {
       set_state("RUNNING") ;
    }
    else if (cmdname == "pause") {
       set_state("PAUSED") ; // FIXME: to be taken from cmd_data 
    }
  } 
  catch(CommandDispatchingFailed & ex) {
    m_busy.store(false);
    m_error.store(true);
    throw ex;
  }
}

void
Application::gather_stats(opmonlib::InfoCollector & ci, int level) 
{
  // TODO:Fill application info and add it to ci
  appinfo::Info ai;
  //ai.partition_name = m_partition; 
  //ai.app_name = get_name();
  ai.state = get_state();
  ai.busy = m_busy.load();
  ai.error = m_error.load();

  opmonlib::InfoCollector tmp_ci;

  tmp_ci.add(ai);

  if (level == 0) {
    // give only generic application info
  } 
  else {
    try {
       m_mod_mgr.gather_stats(tmp_ci, level);
    }
    catch(ers::Issue &ex) {
      ers::error(ex);
    }
  }
  ci.add(get_name(), tmp_ci);
}

bool 
Application::is_cmd_valid(const dataobj_t& cmd_data)
{
  if (m_busy.load() || m_error.load()) 
    return false;

  std::string state = get_state();
  std::string cmd = cmd_data.get<cmdlib::cmd::Command>().id;
  if( (state == "NONE" && cmd == "init") || (state == "INITIAL" && cmd == "conf") 
      || (state == "CONFIGURED" && (cmd == "start" || cmd == "scrap"))
      || (state == "RUNNING" && (cmd == "resume" || cmd == "stop" || cmd == "pause"))
      || (state == "PAUSED" && (cmd == "resume" || cmd == "stop")) ) {
     return true;
  }
  if (!(cmd=="init" || cmd=="conf" || cmd=="start" || cmd=="stop" || cmd == "pause" || cmd == "resume" || cmd == "scrap"))
	return true;

  return false;
}

} // namespace appfwk
} // namespace dunedaq
