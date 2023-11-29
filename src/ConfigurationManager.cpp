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
ConfigurationManager::initialise(std::string& configSpec,
                                 std::string& appName,
                                 std::string& sessionName)
{
  TLOG() << "configSpec <" << configSpec
         << "> session name " << sessionName
         << " application name " << appName;

  m_oksConfigSpec = configSpec;
  m_appName = appName;
  m_sessionName = sessionName;

  m_confdb.reset(new oksdbinterfaces::Configuration(configSpec));

  TLOG_DBG(5) << "getting session";
  m_session = m_confdb->get<coredal::Session>(sessionName);
  if (m_session == nullptr) {
    // Throw an ers Issue here!!
    TLOG() << "Failed to get session";
    exit(0);
  }
  
  TLOG_DBG(5) << "getting app";
  m_application = m_confdb->get<coredal::DaqApplication>(appName);
  if (m_application == nullptr) {
    // Throw an ers Issue here!!
    TLOG() << "Failed to get app";
    exit(0);
  }
}

