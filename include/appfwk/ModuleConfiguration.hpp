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

  class ModuleConfiguration {
    std::vector<const dunedaq::coredal::DaqModule*> m_modules;
    iomanager::Queues_t m_queues;
    iomanager::Connections_t m_networkconnections;
    static std::shared_ptr<ModuleConfiguration> s_instance;
    ModuleConfiguration(){}
  public:
    void initialise();
    static std::shared_ptr<ModuleConfiguration> get() {
      if (!s_instance) {
        s_instance.reset(new ModuleConfiguration);
      }
      return s_instance;
    }
    const iomanager::Queues_t& queues() {return m_queues;}
    const iomanager::Connections_t& networkconnections() {
      return m_networkconnections;
    }
    const std::vector<const coredal::DaqModule*>& modules() {return m_modules;}

    template<typename T> const T* module(const std::string& name){
      return ConfigurationManager::get()->get_dal<T>(name);
    }
  };

} // namespace appfwk
} // namespace dunedaq

#endif // APPFWK_INCLUDE_MODULECONFIGURATION_HPP_
