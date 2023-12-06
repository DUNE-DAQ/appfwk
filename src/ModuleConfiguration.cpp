/**
 * @file ModuleConfiguration.cpp ModuleConfiguration class
 * implementation
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2023.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/ModuleConfiguration.hpp"
#include "oksdbinterfaces/Configuration.hpp"
#include "coredal/Session.hpp"
#include "coredal/DaqApplication.hpp"
#include "coredal/DaqModule.hpp"
#include "coredal/NetworkConnection.hpp"
#include "coredal/Queue.hpp"
#include "coredal/ResourceSet.hpp"
#include "coredal/Service.hpp"
#include "appdal/SmartDaqApplication.hpp"

#include <ifaddrs.h>
#include <netdb.h>
#include <cerrno>


using namespace dunedaq::appfwk;

std::shared_ptr<ModuleConfiguration> ModuleConfiguration::s_instance;

void
ModuleConfiguration::initialise()
{
  auto cfMgr = ConfigurationManager::get();
  auto session = cfMgr->session();
  auto application = cfMgr->application();
  std::shared_ptr<oksdbinterfaces::Configuration> confdb = cfMgr->m_confdb;


  TLOG_DBG(5) << "getting modules";
  auto daqApp = application->cast<coredal::DaqApplication>();
  if (daqApp) {
    m_modules = daqApp->get_modules();
  }
  auto smartDaqApp = application->cast<appdal::SmartDaqApplication>();
  if (smartDaqApp) {
    std::string oksFile = cfMgr->m_oksConfigSpec.substr(9); // Strip off "oksconfig:"
    m_modules = smartDaqApp->generate_modules(confdb.get(), oksFile, session);
  }
  auto resSet = application->cast<coredal::ResourceSet>();
  if (resSet) {
    auto resources = resSet->get_contains();
    for (auto resiter=resources.begin(); resiter!=resources.end(); ++resiter) {
      auto res = *resiter;
      if (!res->disabled(*session)) {
        auto mod = res->cast<coredal::DaqModule>();
        if (mod) {
          m_modules.push_back(mod);
        }
        else {
          ers::warning(NotADaqModule(ERS_HERE,res->UID()));
        }
      }
      else {
        TLOG() << "Ignoring disabled resource " << res->UID();
      }
    }
  }
  std::set<std::string> connectionsAdded;
  for (auto mod : m_modules) {
    TLOG() << "initialising " << mod->class_name() << " module " << mod->UID();
    auto connections = mod->get_inputs();
    auto outputs = mod->get_outputs();
    connections.insert(connections.end(), outputs.begin(), outputs.end());
    for (auto con: connections) {
      auto [c, inserted] = connectionsAdded.insert(con->UID());
      if (!inserted) {
        // Already handled this connection, don't add it again
        continue;
      }
      auto queue = confdb->cast<coredal::Queue>(con);
      if (queue) {
        TLOG() << "Adding queue " << queue->UID();
        m_queues.emplace_back(iomanager::QueueConfig{
            {queue->UID(), queue->get_data_type()},
            iomanager::parse_QueueType(queue->get_queue_type()),
            queue->get_capacity()});
      }
      auto netCon = confdb->cast<coredal::NetworkConnection>(con);
      if (netCon) {
        TLOG() << "Adding network connection " << netCon->UID();
        auto service = netCon->get_associated_service();
        std::string port = "*";
        if (service->get_port()) {
          port = std::to_string(service->get_port());
        }
        std::string ipaddr = "0.0.0.0";
        auto iface = service->get_eth_device_name();
        if (iface != "") {
          // Work out which ip address goes with this device
          struct ifaddrs *ifaddr;
          getifaddrs(&ifaddr);
          for (auto ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == NULL) {
              continue;
            }
            if (std::string(ifa->ifa_name) == iface) {
              char ip[NI_MAXHOST];
              int status = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), ip, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
              if (status != 0) {
                continue;
              }
              ipaddr = std::string(ip);
              break;
            }
          }
        }
        std::string uri(service->get_protocol() + "://" + ipaddr + ":" + port);
        m_networkconnections.emplace_back(iomanager::Connection{
            {netCon->UID(), con->get_data_type()},
            uri,
            iomanager::parse_ConnectionType(netCon->get_connection_type())
          });
      }
    }
  }
}
