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

ConfigurationManager::ConfigurationManager(const std::string& config_spec, const std::string& app_name)
{
  TLOG() << "configSpec <" << config_spec << "> application name " << app_name;

  std::string delimiter = ":";
  std::vector<std::string> tokens;
  std::string s = config_spec;
  size_t pos = 0;
  while ((pos = s.find(delimiter)) != std::string::npos) {
    std::string token = s.substr(0, pos);
    tokens.push_back(token);
    s.erase(0, pos + delimiter.length());
  }
  tokens.push_back(s);

  for (auto& token : tokens) {
    TLOG() << "token: " << token;
  }

  m_system_name = *tokens.rbegin();
  s = config_spec;
  pos = s.find(m_system_name);
  m_oks_config_spec = s.substr(0, pos-1);
  m_app_name = app_name;
  TLOG() << "m_system_name: " << m_system_name;
  TLOG() << "m_oks_config_spec: " << m_oks_config_spec;
  TLOG() << "m_app_name: " << m_app_name;

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
