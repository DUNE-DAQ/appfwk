
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

    const iomanager::Queues_t& queues();
    const iomanager::Connections_t& networkconnections();
    const std::vector<const dunedaq::coredal::DaqModule*>& modules();
  };

} // namespace dunedaq::appfwk
