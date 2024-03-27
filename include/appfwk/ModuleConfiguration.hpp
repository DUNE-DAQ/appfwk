/**
 * @file ModuleConfiguration.hpp ModuleConfiguration class definition
 *
 *
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_MODULECONFIGURATION_HPP_
#define APPFWK_INCLUDE_MODULECONFIGURATION_HPP_

#include "appfwk/ConfigurationManager.hpp"
#include "coredal/ActionPlan.hpp"
#include "coredal/DaqModule.hpp"
#include "iomanager/IOManager.hpp"
#include "oksdbinterfaces/Configuration.hpp"

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

namespace coredal {
class DaqModule;
class Session;
class Application;
}
namespace appfwk {

class ModuleConfiguration
{
  std::shared_ptr<ConfigurationManager> m_config_mgr;
  std::vector<const dunedaq::coredal::DaqModule*> m_modules;
  std::unordered_map<std::string, const dunedaq::coredal::ActionPlan*> m_action_plans;
  iomanager::Queues_t m_queues;
  iomanager::Connections_t m_networkconnections;

public:
  explicit ModuleConfiguration(std::shared_ptr<ConfigurationManager> mgr);

  const iomanager::Queues_t& queues() { return m_queues; }
  const iomanager::Connections_t& networkconnections() { return m_networkconnections; }
  const std::vector<const coredal::DaqModule*>& modules() { return m_modules; }

  const std::unordered_map<std::string, const dunedaq::coredal::ActionPlan*>& action_plans() { return m_action_plans; }
  const dunedaq::coredal::ActionPlan* action_plan(std::string cmd) const;

  std::shared_ptr<ConfigurationManager> configuration_manager() { return m_config_mgr; }

  template<typename T>
  const T* module(const std::string& name)
  {
    return m_config_mgr->get_dal<T>(name);
  }
};

} // namespace appfwk
} // namespace dunedaq

#endif // APPFWK_INCLUDE_MODULECONFIGURATION_HPP_
