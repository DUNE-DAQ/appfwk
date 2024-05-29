/**
 * @file ModuleConfiguration.hpp ModuleConfiguration class definition
 *
 *
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_CONFIGURATIONMANAGER_HPP_
#define APPFWK_INCLUDE_CONFIGURATIONMANAGER_HPP_

#include "coredal/DaqModule.hpp"
#include "iomanager/IOManager.hpp"
#include "conffwk/Configuration.hpp"

#include <string>
#include <vector>

namespace dunedaq {

namespace coredal {
class Session;
class Application;
}
namespace appfwk {

class ConfigurationManager
{
  friend class ModuleConfiguration;

  std::shared_ptr<conffwk::Configuration> m_confdb;
  std::string m_app_name;
  std::string m_session_name;
  std::string m_oks_config_spec;
  const coredal::Session* m_session;
  const coredal::Application* m_application;

public:
  ConfigurationManager(std::string& config_spec, std::string& app_name, std::string& session_name);

  const coredal::Session* session() { return m_session; }
  const coredal::Application* application() { return m_application; }
  template<typename T>
  const T* get_dal(const std::string& name)
  {
    return m_confdb->get<T>(name);
  }
};

} // namespace appfwk
} // namespace dunedaq

#endif // APPFWK_INCLUDE_CONFIGURATIONMANAGER_HPP_
