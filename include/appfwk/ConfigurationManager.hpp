/**
 * @file ConfigurationManager.hpp ConfigurationManager class definition
 *
 *
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_CONFIGURATIONMANAGER_HPP_
#define APPFWK_INCLUDE_CONFIGURATIONMANAGER_HPP_

#include "conffwk/Configuration.hpp"
#include "confmodel/ActionPlan.hpp"
#include "confmodel/DaqModule.hpp"
#include "confmodel/ResourceBase.hpp"
#include "iomanager/IOManager.hpp"
#include "logging/Logging.hpp" // NOTE: if ISSUES ARE DECLARED BEFORE include logging/Logging.hpp, TLOG_DEBUG<<issue wont work.

#include <string>
#include <vector>

namespace dunedaq {

ERS_DECLARE_ISSUE(appfwk,                                                                  ///< Namespace
                  NotADaqModule,                                                           ///< Issue class name
                  "Application contains a resource " << res << " that is not a DaqModule", ///< Message
                  ((std::string)res)                                                       ///< Message parameters
)
ERS_DECLARE_ISSUE(appfwk,             ///< Namespace
                  NotADaqApplication, ///< Issue class name
                  "Application " << app << " is neither a DaqApplication nor a SmartDaqApplication ", ///< Message
                  ((std::string)app) ///< Message parameters
)
namespace confmodel {
class Session;
class Application;
class DaqModule;
}
namespace appfwk {

class ConfigurationManager
{
  std::shared_ptr<conffwk::Configuration> m_confdb;
  std::string m_app_name;
  std::string m_session_name;
  std::string m_oks_config_spec;
  const confmodel::Session* m_session;
  const confmodel::Application* m_application;
  std::unordered_map<std::string, const confmodel::ActionPlan*> m_action_plans;
  std::vector<const confmodel::DaqModule*> m_modules;
  std::vector<const confmodel::Queue*> m_queues;
  std::vector<const confmodel::NetworkConnection*> m_networkconnections;
  const confmodel::ConnectivityService* m_connsvc_config;
  bool initialized{ false };

public:

  ConfigurationManager(std::string& config_spec, std::string& app_name, std::string& session_name);
  void initialize();

  const confmodel::Session* session()
  {
    return m_session;
  }
  const confmodel::Application* application()
  {
    initialize();
    return m_application;
  }

  const std::vector<const confmodel::Queue*>& queues()
  {
    initialize();
    return m_queues;
  }
  const std::vector<const confmodel::NetworkConnection*>& networkconnections()
  {
    initialize();
    return m_networkconnections;
  }
  const std::vector<const confmodel::DaqModule*>& modules()
  {
    initialize();
    return m_modules;
  }
  const confmodel::ConnectivityService* connectivity_service()
  {
    initialize();
    return m_connsvc_config;
  }

  const std::unordered_map<std::string, const confmodel::ActionPlan*>& action_plans()
  {
    initialize();
    return m_action_plans;
  }
  const confmodel::ActionPlan* action_plan(std::string cmd) const;

  template<typename T>
  const T* get_dal(const std::string& name)
  {
    return m_confdb->get<T>(name);
  }
};

} // namespace appfwk
} // namespace dunedaq

#endif // APPFWK_INCLUDE_CONFIGURATIONMANAGER_HPP_
