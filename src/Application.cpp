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

#include "ers/ers.h"

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

  if(!is_cmd_valid(cmd_data)) {
    std::string cmdname = cmd_data.get<cmd::Command>().id;
    throw InvalidCommand(ERS_HERE, cmdname, m_state, m_error.load(), m_busy.load());
  }

  m_busy.store(true);

  try {
    m_mod_mgr.execute(cmd_data);
    m_busy.store(false);
    m_state = "ANY" ; // FIXME: to be taken from cmd_data 
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

  if (level == 0) {
    // give only generic application info
  } 
  else {
    try {
       m_mod_mgr.gather_stats(ci, level);
    }
    catch(ers::Issue &ex) {
      ers::error(ex);
    }
  }
}

bool 
Application::is_cmd_valid(const dataobj_t& /*cmd_data*/)
{
 bool invalid = m_error.load(); 
  return !invalid; //FIXME: validity of command to be checked against state!
}

} // namespace appfwk
} // namespace dunedaq
