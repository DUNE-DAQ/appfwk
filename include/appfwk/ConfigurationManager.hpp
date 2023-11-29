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

#include <string>
#include <vector>

namespace dunedaq {

namespace coredal {
  class Session;
  class Application;
}
namespace appfwk {

  class ConfigurationManager {
    std::shared_ptr<oksdbinterfaces::Configuration> m_confdb;
    std::string m_appName;
    std::string m_sessionName;
    std::string m_oksConfigSpec;
    const coredal::Session* m_session;
    const coredal::Application* m_application;
    static std::shared_ptr<ConfigurationManager> s_instance;

  public:
    static std::shared_ptr<ConfigurationManager> get() {
      if (!s_instance) {
        s_instance.reset(new ConfigurationManager);
      }
      return s_instance;
    }
    void initialise(std::string& configSpec,
                    std::string& appName,
                    std::string& sessionName) ;
    std::shared_ptr<oksdbinterfaces::Configuration> confdb() {return m_confdb;}
    const coredal::Session* session() {return m_session;}
    const coredal::Application* application() {return m_application;}
    template<typename T> const T* get_dal(const std::string& name) {return m_confdb->get<T>(name);}
    std::string oks_config_spec() {return m_oksConfigSpec;}
  };

} // namespace appfwk
} // namespace dunedaq

#endif // APPFWK_INCLUDE_CONFIGURATIONMANAGER_HPP_
