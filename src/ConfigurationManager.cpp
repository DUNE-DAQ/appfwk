/**
 * @file ConfigurationManager.cpp ConfigurationManager class
 * implementation
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2023.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/ConfigurationManager.hpp"
#include "appfwk/Issues.hpp"
#include "appmodel/SmartDaqApplication.hpp"
#include "conffwk/Configuration.hpp"
#include "confmodel/DaqApplication.hpp"
#include "confmodel/DaqModule.hpp"
#include "confmodel/DaqModulesGroupByType.hpp"
#include "confmodel/FSMCommand.hpp"
#include "confmodel/NetworkConnection.hpp"
#include "confmodel/Queue.hpp"
#include "confmodel/ResourceSet.hpp"
#include "confmodel/Service.hpp"
#include "confmodel/Session.hpp"

using namespace dunedaq::appfwk;

ConfigurationManager::ConfigurationManager(std::string& config_spec, std::string& app_name, std::string& session_name)
{
  TLOG() << "configSpec <" << config_spec << "> session name " << session_name << " application name " << app_name;

  m_oks_config_spec = config_spec;
  m_app_name = app_name;
  m_session_name = session_name;

  m_confdb.reset(new conffwk::Configuration(config_spec));

  TLOG_DBG(5) << "getting session";
  m_session = m_confdb->get<confmodel::Session>(session_name);
  if (m_session == nullptr) {
    TLOG() << "Failed to get session";
    throw MissingComponent(ERS_HERE, "Session " + session_name);
  }
}

void
ConfigurationManager::initialize()
{
  if (initialized) {
    return;
  }
  TLOG_DBG(5) << "getting app";
  m_application = m_confdb->get<confmodel::Application>(m_app_name);
  if (m_application == nullptr) {
    TLOG() << "Failed to get app";
    throw MissingComponent(ERS_HERE, "Application " + m_app_name);
  }

  TLOG_DBG(5) << "getting modules";
  auto smartDaqApp = m_application->cast<appmodel::SmartDaqApplication>();
  if (smartDaqApp) {
    auto cpos = m_oks_config_spec.find(":") + 1;
    std::string oksFile = m_oks_config_spec.substr(cpos); // Strip off "oksconflibs:"
    m_modules = smartDaqApp->generate_modules(m_confdb.get(), oksFile, m_session);

    for (auto& plan : smartDaqApp->get_action_plans()) {
      auto cmd = plan->get_command()->get_cmd();
      TLOG_DBG(6) << "Registering action plan " << plan->UID() << " for cmd " << cmd;
      if (m_action_plans.count(cmd)) {
        throw ActionPlanValidationFailed(
          ERS_HERE, cmd, "N/A", "Multiple ActionPlans registered for cmd, conflicting plan is " + plan->UID());
      }
      for (auto& step : plan->get_steps()) {
        auto step_byType = step->cast<confmodel::DaqModulesGroupByType>();
        if (step_byType == nullptr) {
          throw ActionPlanValidationFailed(
            ERS_HERE, cmd, "N/A", "ActionPlans for SmartDaqApplications must use DaqModulesGroupByType");
        }
      }
      m_action_plans[cmd] = plan;
    }
  } else {
    auto daqApp = m_application->cast<confmodel::DaqApplication>();
    if (daqApp) {
      m_modules = daqApp->get_modules();

      for (auto& plan : daqApp->get_action_plans()) {
        auto cmd = plan->get_command()->get_cmd();
        TLOG_DBG(6) << "Registering action plan " << plan->UID() << " for cmd " << cmd;
        if (m_action_plans.count(cmd)) {
          throw ActionPlanValidationFailed(
            ERS_HERE, cmd, "N/A", "Multiple ActionPlans registered for cmd, conflicting plan is " + plan->UID());
        }
        m_action_plans[cmd] = plan;
      }
    } else {
      throw(NotADaqApplication(ERS_HERE, m_application->UID()));
    }
  }

  m_connsvc_config = m_session->get_connectivity_service();

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
      auto queue = m_confdb->cast<confmodel::Queue>(con);
      if (queue) {
        TLOG() << "Adding queue " << queue->UID();
        m_queues.emplace_back(queue);
      }
      auto netCon = m_confdb->cast<confmodel::NetworkConnection>(con);
      if (netCon) {
        m_networkconnections.emplace_back(netCon);
      }
    }
  }

  initialized = true;
}

const dunedaq::confmodel::ActionPlan*
ConfigurationManager::action_plan(std::string cmd) const
{
  if (m_action_plans.count(cmd)) {
    return m_action_plans.at(cmd);
  }
  return nullptr;
}
