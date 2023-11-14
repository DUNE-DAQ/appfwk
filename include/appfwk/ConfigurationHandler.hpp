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

namespace dunedaq::coredal {
  class DaqModule;
  class Session;
  class Application;
}
namespace dunedaq::appfwk {

  class ConfigurationHandler {
    oksdbinterfaces::Configuration m_confdb;
    const std::string m_appName;
    const std::string m_sessionName;
    const coredal::Session* m_session;
    const coredal::Application* m_application;
    std::vector<const dunedaq::coredal::DaqModule*> m_modules;
    iomanager::Queues_t m_queues;
    iomanager::Connections_t m_networkconnections;

  public:
    ConfigurationHandler(std::string& configSpec,
                         std::string& appName,
                         std::string& sessionName);

    const iomanager::Queues_t& queues() {return m_queues;}
    const iomanager::Connections_t& networkconnections() {
      return m_networkconnections;
    }
    const coredal::Session* session() {return m_session;}
    const coredal::Application* application() {return m_application;}
    const std::vector<const coredal::DaqModule*>& modules() {return m_modules;}
    const coredal::DaqModule* module(const std::string& name){
      return m_confdb.get<coredal::DaqModule>(name);
    }
  };

} // namespace dunedaq::appfwk
