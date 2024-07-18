/**
 * @file ModuleConfiguration.cpp ModuleConfiguration class
 * implementation
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2023.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/ModuleConfiguration.hpp"
#include "appmodel/SmartDaqApplication.hpp"
#include "confmodel/DaqApplication.hpp"
#include "confmodel/DaqModule.hpp"
#include "confmodel/NetworkConnection.hpp"
#include "confmodel/Queue.hpp"
#include "confmodel/ResourceSet.hpp"
#include "confmodel/Service.hpp"
#include "confmodel/Session.hpp"
#include "conffwk/Configuration.hpp"

#include <cerrno>
#include <ifaddrs.h>
#include <netdb.h>

using namespace dunedaq::appfwk;

ModuleConfiguration::ModuleConfiguration(std::shared_ptr<ConfigurationManager> cfMgr)
  : m_config_mgr(cfMgr)
  , m_action_plans()
{
  auto session = cfMgr->session();
  auto application = cfMgr->application();
  std::shared_ptr<conffwk::Configuration> confdb = cfMgr->m_confdb;

  TLOG_DBG(5) << "getting modules";
  auto smartDaqApp = application->cast<appmodel::SmartDaqApplication>();
  if (smartDaqApp) {
    auto cpos = cfMgr->m_oks_config_spec.find(":") + 1;
    std::string oksFile = cfMgr->m_oks_config_spec.substr(cpos); // Strip off "oksconflibs:"
    m_modules = smartDaqApp->generate_modules(confdb.get(), oksFile, session);

    for (auto& plan : smartDaqApp->get_action_plans()) {
      TLOG_DBG(6) << "Registering action plan " << plan->UID();
      m_action_plans[plan->UID()] = plan;
    }
  } else {
    auto daqApp = application->cast<confmodel::DaqApplication>();
    if (daqApp) {
      m_modules = daqApp->get_modules();

      for (auto& plan : daqApp->get_action_plans()) {
        TLOG_DBG(6) << "Registering action plan " << plan->UID();
        m_action_plans[plan->UID()] = plan;
      }
    } else {
      throw(NotADaqApplication(ERS_HERE, application->UID()));
    }
  }

  std::set<std::string> connectionsAdded;
  for (auto mod : m_modules) {
    TLOG() << "initialising " << mod->class_name() << " module " << mod->UID();
    auto connections = mod->get_inputs();
    auto outputs = mod->get_outputs();
    connections.insert(connections.end(), outputs.begin(), outputs.end());
    for (auto con : connections) {
      auto [c, inserted] = connectionsAdded.insert(con->UID());
      if (!inserted) {
        // Already handled this connection, don't add it again
        continue;
      }
      auto queue = confdb->cast<confmodel::Queue>(con);
      if (queue) {
        TLOG() << "Adding queue " << queue->UID();
        m_queues.emplace_back(iomanager::QueueConfig{ { queue->UID(), queue->get_data_type() },
                                                      iomanager::parse_QueueType(queue->get_queue_type()),
                                                      queue->get_capacity() });
      }
      auto netCon = confdb->cast<confmodel::NetworkConnection>(con);
      if (netCon) {
        TLOG() << "Adding network connection " << netCon->UID();
        auto service = netCon->get_associated_service();
        std::string port = "*";
        if (service->get_port()) {
          port = std::to_string(service->get_port());
        }
        std::string ipaddr = "0.0.0.0";
        char hostname[256];
        if (gethostname(&hostname[0], 256) == 0) {
          ipaddr = std::string(hostname);
        }
        auto iface = service->get_eth_device_name();
        if (iface != "") {
          // Work out which ip address goes with this device
          struct ifaddrs* ifaddr;
          getifaddrs(&ifaddr);
          for (auto ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa->ifa_addr == NULL) {
              continue;
            }
            if (std::string(ifa->ifa_name) == iface) {
              char ip[NI_MAXHOST];
              int status =
                getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), ip, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);
              if (status != 0) {
                continue;
              }
              ipaddr = std::string(ip);
              break;
            }
          }
        }
        std::string uri(service->get_protocol() + "://" + ipaddr + ":" + port);
        m_networkconnections.emplace_back(
          iomanager::Connection{ { netCon->UID(), con->get_data_type() },
                                 uri,
                                 iomanager::parse_ConnectionType(netCon->get_connection_type()) });
      }
    }
  }
}

const dunedaq::coredal::ActionPlan*
dunedaq::appfwk::ModuleConfiguration::action_plan(std::string cmd) const
{
  if (m_action_plans.count(cmd)) {
    return m_action_plans.at(cmd);
  }
  return nullptr;
}
