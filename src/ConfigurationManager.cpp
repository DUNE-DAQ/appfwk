/**
 * @file ConfigurationManager.cpp ConfigurationManager class
 * implementation
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2023.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/ConfigurationManager.hpp"
#include "coredal/DaqApplication.hpp"
#include "coredal/Session.hpp"
#include "oksdbinterfaces/Configuration.hpp"

using namespace dunedaq::appfwk;

ConfigurationManager::ConfigurationManager(std::string& config_spec,
                                           std::string& app_name,
                                           std::string& session_name)
  : m_app_name(app_name)
  , m_session_name(session_name)
  , m_oks_config_spec(config_spec)
  , m_application(nullptr)
{
  TLOG() << "configSpec <" << m_oks_config_spec << "> session name "
         << m_session_name << " application name " << m_app_name;
  m_confdb.reset(new oksdbinterfaces::Configuration(config_spec));

  TLOG_DBG(5) << "getting session";
  m_session = m_confdb->get<coredal::Session>(session_name);
  if (m_session == nullptr) {
    // Throw an ers Issue here!!
    TLOG() << "Failed to get session";
    exit(0);
  }

  TLOG_DBG(5) << "getting app";
  // m_application = m_confdb->get<coredal::Application>(app_name);
  for (auto app: m_session->get_all_applications()) {
    if (app->UID() == app_name) {
      m_application = app;
      break;
    }
  }
  if (m_application == nullptr) {
    // Throw an ers Issue here!!
    TLOG() << "Failed to get app";
    exit(0);
  }
}
