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
    friend class ModuleConfiguration;

    std::shared_ptr<oksdbinterfaces::Configuration> m_confdb;
    std::string m_appName;
    std::string m_sessionName;
    std::string m_oksConfigSpec;
    const coredal::Session* m_session;
    const coredal::Application* m_application;

    static std::shared_ptr<ConfigurationManager> s_instance;
    ConfigurationManager(){}
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

    const coredal::Session* session() {return m_session;}
    const coredal::Application* application() {return m_application;}
    template<typename T> const T* get_dal(const std::string& name) {return m_confdb->get<T>(name);}
  };

} // namespace appfwk
} // namespace dunedaq

#endif // APPFWK_INCLUDE_CONFIGURATIONMANAGER_HPP_