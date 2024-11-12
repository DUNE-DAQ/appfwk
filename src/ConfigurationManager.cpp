/**
 * @file ConfigurationManager.cpp ConfigurationManager class
 * implementation
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2023.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/ConfigurationManager.hpp"
#include "confmodel/DaqApplication.hpp"
#include "confmodel/System.hpp"
#include "conffwk/Configuration.hpp"

using namespace dunedaq::appfwk;

ConfigurationManager::ConfigurationManager(const std::string& config_spec, const std::string& system_name, const std::string& app_name)
: m_confdb(nullptr)
, m_app_name(app_name)
, m_system_name(system_name)
, m_oks_config_spec(config_spec)
, m_system(nullptr)
, m_application(nullptr)
{
  TLOG() << "configSpec <" << m_oks_config_spec << "> system " << m_system_name << " application name " << app_name;

  m_confdb.reset(new conffwk::Configuration(m_oks_config_spec));

  TLOG_DBG(5) << "getting system";
  m_system = m_confdb->get<confmodel::System>(m_system_name);
  if (m_system == nullptr) {
    // Throw an ers Issue here!!
    TLOG() << "Failed to get the system";
    exit(0);
  }

  TLOG_DBG(5) << "getting app";
  m_application = m_confdb->get<confmodel::Application>(app_name);
  if (m_application == nullptr) {
    // Throw an ers Issue here!!
    TLOG() << "Failed to get app";
    exit(0);
  }
}
