/**
 * @file ConfigurationManager.cpp ConfigurationManager class
 * implementation
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2023.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/ConfigurationManager.hpp"

#include "appmodel/SmartDaqApplication.hpp"
#include "conffwk/Configuration.hpp"
#include "confmodel/DaqApplication.hpp"
#include "confmodel/DaqModule.hpp"
#include "confmodel/DaqModulesGroupByType.hpp"
#include "confmodel/FSMCommand.hpp"
#include "confmodel/NetworkConnection.hpp"
#include "confmodel/Queue.hpp"
#include "confmodel/Service.hpp"
#include "confmodel/Session.hpp"

#include <set>
#include <string>

using namespace dunedaq::appfwk;

enum
{
  TLVL_SESSION = 5,
  TLVL_APP = 6,
  TLVL_MODULE = 7,
  TLVL_QUEUE = 8,
  TLVL_ACTION_PLAN = 9,

};

ConfigurationManager::ConfigurationManager(std::string const& config_spec,
                                           std::string const& app_name,
                                           std::string const& session_name)
  : m_confdb(new conffwk::Configuration(config_spec))
  , m_app_name(app_name)
  , m_session_name(session_name)
{
  TLOG() << "configSpec <" << config_spec << "> session name " << session_name << " application name " << app_name;

  TLOG_DBG(TLVL_SESSION) << "getting session " << session_name;
  m_session = m_confdb->get<confmodel::Session>(session_name);
  if (m_session == nullptr) {
    TLOG() << "Failed to get session " << session_name;
    throw MissingComponent(ERS_HERE, "Session " + session_name);
  }
  m_helper = std::make_shared<appmodel::ConfigurationHelper>(m_session);
}

std::vector<ValidationReport>
ConfigurationManager::initialize(bool throw_on_fatal)
{
  std::vector<ValidationReport> reports;
  if (m_initialized) {
    return reports;
  }
  TLOG_DBG(TLVL_APP) << "getting app " << m_app_name;
  m_application = m_confdb->get<confmodel::DaqApplication>(m_app_name);
  if (m_application == nullptr) {
    TLOG() << "Failed to get app " << m_app_name;
    throw MissingComponent(ERS_HERE, "Application " + m_app_name);
  }

  TLOG_DBG(TLVL_APP) << "getting modules for app " << m_app_name;
  auto daq_app = m_application->cast<confmodel::DaqApplication>();
  if(daq_app == nullptr) {
    throw(NotADaqApplication(ERS_HERE, m_application->UID()));    
  }

  auto smart_daq_app = m_application->cast<appmodel::SmartDaqApplication>();
  if (smart_daq_app != nullptr) {
    smart_daq_app->generate_modules(m_helper);
  }
 
  m_modules = m_application->get_modules();
   
  for (auto& plan : m_application->get_action_plans()) {
    auto cmd = plan->get_command()->get_cmd();
    TLOG_DBG(TLVL_ACTION_PLAN) << "Registering action plan " << plan->UID() << " for cmd " << cmd;
    if (m_action_plans.count(cmd)) {
      reports.emplace_back(ValidationReport::Severity::Fatal,
                           m_app_name,
                           "N/A",
                           cmd,
                           "Multiple ActionPlans registered for cmd, conflicting plan is " + plan->UID());
      if (throw_on_fatal)
        throw ActionPlanValidationFailed(
          ERS_HERE, reports.back().get_command(), reports.back().get_module(), reports.back().get_message());
      else
        ers::error(ActionPlanValidationFailed(
          ERS_HERE, reports.back().get_command(), reports.back().get_module(), reports.back().get_message()));
    }
    m_action_plans[cmd] = plan;
  }

  m_connsvc_config = m_session->get_connectivity_service();

  std::set<std::string> connectionsAdded;
  for (auto mod : m_modules) {
    TLOG_DBG(TLVL_MODULE) << "initialising " << mod->class_name() << " module " << mod->UID();
    auto connections = mod->get_inputs();
    auto outputs = mod->get_outputs();
    connections.insert(connections.end(), outputs.begin(), outputs.end());
    for (auto con : connections) {
      auto [c, inserted] = connectionsAdded.insert(con->UID());
      if (!inserted) {
        // Already handled this connection, don't add it again
        continue;
      }
      auto queue = m_confdb->cast<confmodel::Queue>(con);
      if (queue) {
        TLOG_DBG(TLVL_QUEUE) << "Adding queue " << queue->UID();
        m_queues.emplace_back(queue);
      }
      auto net_con = m_confdb->cast<confmodel::NetworkConnection>(con);
      if (net_con) {
        m_networkconnections.emplace_back(net_con);
      }
    }
  }

  m_initialized = true;
  return reports;
}

const dunedaq::confmodel::ActionPlan*
ConfigurationManager::get_action_plan(std::string cmd) const
{
  if (m_action_plans.count(cmd)) {
    return m_action_plans.at(cmd);
  }
  return nullptr;
}
