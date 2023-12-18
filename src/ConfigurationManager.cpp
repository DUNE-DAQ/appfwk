/**
 * @file ConfigurationManager.cpp ConfigurationManager class
 * implementation
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2023.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/ConfigurationManager.hpp"
#include "oksdbinterfaces/Configuration.hpp"
#include "coredal/Session.hpp"
#include "coredal/DaqApplication.hpp"

using namespace dunedaq::appfwk;

std::shared_ptr<ConfigurationManager> ConfigurationManager::s_instance;

void
ConfigurationManager::initialise(std::string& config_spec,
                                 std::string& app_name,
                                 std::string& session_name)
{
  TLOG() << "configSpec <" << config_spec << "> session name " << session_name << " application name " << app_name;

  m_oks_config_spec = config_spec;
  m_app_name = app_name;
  m_session_name = session_name;

  m_confdb.reset(new oksdbinterfaces::Configuration(config_spec));

  TLOG_DBG(5) << "getting session";
  m_session = m_confdb->get<coredal::Session>(session_name);
  if (m_session == nullptr) {
    // Throw an ers Issue here!!
    TLOG() << "Failed to get session";
    exit(0);
  }
  
  TLOG_DBG(5) << "getting app";
  m_application = m_confdb->get<coredal::DaqApplication>(app_name);
  if (m_application == nullptr) {
    // Throw an ers Issue here!!
    TLOG() << "Failed to get app";
    exit(0);
  }
}

