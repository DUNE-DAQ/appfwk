/**
 * @file ConfigurationHandler.hpp ConfigurationHandler class definition
 *
 * 
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "coredal/DaqModule.hpp"
#include "iomanager/IOManager.hpp"
#include "oksdbinterfaces/Configuration.hpp"

#include <string>
#include <vector>

namespace dunedaq {

ERS_DECLARE_ISSUE(appfwk,                                                   ///< Namespace
                  NotADaqModule,                                            ///< Issue class name
                  "Application contains a resource " << res << " that is not a DaqModule", ///< Message
                  ((std::string)res)                                      ///< Message parameters
)

namespace coredal {
  class DaqModule;
  class Session;
  class Application;
}
namespace appfwk {

  class ConfigurationHandler {
    std::shared_ptr<oksdbinterfaces::Configuration> m_confdb;
    std::string m_appName;
    std::string m_sessionName;
    const coredal::Session* m_session;
    const coredal::Application* m_application;
    std::vector<const dunedaq::coredal::DaqModule*> m_modules;
    iomanager::Queues_t m_queues;
    iomanager::Connections_t m_networkconnections;
    static std::shared_ptr<ConfigurationHandler> s_instance;
    ConfigurationHandler(){}
  public:
    void initialise(std::shared_ptr<oksdbinterfaces::Configuration> confdb,
                    std::string& configSpec,
                    std::string& appName,
                    std::string& sessionName);
    static std::shared_ptr<ConfigurationHandler> get() {
      if (!s_instance) {
        s_instance.reset(new ConfigurationHandler);
      }
      return s_instance;
    }
    const iomanager::Queues_t& queues() {return m_queues;}
    const iomanager::Connections_t& networkconnections() {
      return m_networkconnections;
    }
    const coredal::Session* session() {return m_session;}
    const coredal::Application* application() {return m_application;}
    const std::vector<const coredal::DaqModule*>& modules() {return m_modules;}

    template<typename T> const T* module(const std::string& name){
      return m_confdb->get<T>(name);
    }
  };

} // namespace appfwk
} // namespace dunedaq
