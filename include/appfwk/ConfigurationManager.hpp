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
#include "oksdbinterfaces/Configuration.hpp"
#include "oksdbinterfaces/ConfigObject.hpp"
#include "oksdbinterfaces/Schema.hpp"

#include "nlohmann/json.hpp"

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

  std::shared_ptr<oksdbinterfaces::Configuration> m_confdb;
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


  nlohmann::json get_json_config(const std::string& class_name,
                                 const std::string& uid,
                                 bool direct_only = false);
  template <typename T> void add_json_value(oksdbinterfaces::ConfigObject& obj,
                                            std::string& name,
                                            bool multi_value,
                                            nlohmann::json& attributes) {
    if (!multi_value) {
      T value;
      obj.get(name, value);
      attributes[name] = value;
    }
    else {
      std::vector<T> value_vector;
      obj.get(name, value_vector);
      attributes[name] = nlohmann::json(value_vector);
    }
  }
};

} // namespace appfwk
} // namespace dunedaq

#endif // APPFWK_INCLUDE_CONFIGURATIONMANAGER_HPP_
