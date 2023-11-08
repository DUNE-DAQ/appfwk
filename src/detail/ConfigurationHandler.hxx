#include "ConfigurationHandler.hpp"
#include "oksdbinterfaces/Configuration.hpp"
#include "coredal/Session.hpp"
#include "coredal/DaqApplication.hpp"
#include "coredal/DaqModule.hpp"
#include "coredal/NetworkConnection.hpp"
#include "coredal/Queue.hpp"
#include "coredal/ResourceSet.hpp"
#include "appdal/SmartDaqApplication.hpp"

using namespace dunedaq::appfwk;

ConfigurationHandler::ConfigurationHandler(std::string& configSpec,
                             std::string& appName,
                             std::string& sessionName)
  : m_confdb(configSpec)
  , m_appName(appName)
  , m_sessionName(sessionName)
{
  TLOG() << "session name " << sessionName
         << " application name " << appName;
  TLOG_DBG(5) << "getting session";
  m_session = m_confdb.get<coredal::Session>(sessionName);
  if (m_session == nullptr) {
    // Throw an ers Issue here!!
    TLOG() << "Failed to get session";
    exit(0);
  }
  
  TLOG_DBG(5) << "getting app";
  m_application = m_confdb.get<coredal::DaqApplication>(appName);
  if (m_application == nullptr) {
    // Throw an ers Issue here!!
    TLOG() << "Failed to get app";
    exit(0);
  }

  TLOG_DBG(5) << "getting modules";
  auto daqApp = m_application->cast<coredal::DaqApplication>();
  if (daqApp) {
    m_modules = daqApp->get_modules();
  }
  auto smartDaqApp = m_application->cast<appdal::SmartDaqApplication>();
  if (smartDaqApp) {
    std::string oksFile = configSpec.substr(9); // Strip off "oksconfig:"
    m_modules = smartDaqApp->generate_modules(&m_confdb, oksFile, m_session);
  }
  auto resSet = m_application->cast<coredal::ResourceSet>();
  if (resSet) {
    auto resources = resSet->get_contains();
    for (auto resiter=resources.begin(); resiter!=resources.end(); ++resiter) {
      auto res = *resiter;
      if (!res->disabled(*m_session)) {
        auto mod = res->cast<coredal::DaqModule>();
        if (mod) {
          m_modules.push_back(mod);
        }
        else {
          // ers::warning(NotADaqModule(ERS_HERE,res->UID()));
        }
      }
      else {
        TLOG() << "Ignoring disabled resource " << res->UID();
      }
    }
  }
  for (auto mod : m_modules) {
    TLOG() << "initialising " << mod->class_name() << " module " << mod->UID();
    auto connections = mod->get_inputs();
    auto outputs = mod->get_outputs();
    connections.insert(connections.end(), outputs.begin(), outputs.end());
    for (auto con: connections) {
      auto queue = m_confdb.cast<coredal::Queue>(con);
      if (queue) {
        TLOG() << "Adding queue " << queue->UID();
        m_queues.emplace_back(iomanager::QueueConfig{
            {queue->UID(), queue->get_data_type()},
            iomanager::parse_QueueType(queue->get_queue_type()),
            queue->get_capacity()});
      }
      auto netCon = m_confdb.cast<coredal::NetworkConnection>(con);
      if (netCon) {
        TLOG() << "Adding network connection " << netCon->UID();
        m_networkconnections.emplace_back(iomanager::Connection{
            {netCon->UID(), netCon->get_data_type()},
            netCon->get_uri(),
            iomanager::parse_ConnectionType(netCon->get_connection_type())
          });
      }
    }
  }
}


const dunedaq::iomanager::Queues_t&
ConfigurationHandler::queues() {
  return m_queues;
}

const dunedaq::iomanager::Connections_t&
ConfigurationHandler::networkconnections() {
  return m_networkconnections;
}

const std::vector<const dunedaq::coredal::DaqModule*>&
ConfigurationHandler::modules() {
  return m_modules;
}
