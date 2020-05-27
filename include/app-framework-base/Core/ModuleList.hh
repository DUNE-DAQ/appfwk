/**
 * @file ModuleList class interface
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_CORE_MODULELIST_HH_
#define APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_CORE_MODULELIST_HH_

#include "app-framework-base/DAQModules/DAQModuleI.hh"
#include "app-framework-base/Queues/QueueI.hh"

#include <list>
#include <map>
#include <memory>
#include <string>

namespace appframework {
typedef std::map<std::string, std::unique_ptr<DAQModuleI>>
  DAQModuleMap; ///< DAQModules indexed by name
typedef std::map<std::string, std::shared_ptr<QueueI>>
  QueueMap; ///< Queues indexed by name
typedef std::map<std::string, std::list<std::string>>
  CommandOrderMap; ///< Defined DAQModule orders for commands

/**
 * @brief The ModuleList class is the representation of a DAQModule and Queue
 * graph
 *
 * The ConstructGraph method is responsible for instantiating all Queues and
 * DAQModules for a DAQ Application, as well as linking them together.
 * DAQModules define their Queue endpoints by requiring Queue instances
 */
class ModuleList
{
public:
  /**
   * @brief Construct the graph of DAQModules and Queues.
   * @param[out] buffer_map A QueueMap that will contain pointers to all of the
   * Queue instances, indexed by name
   * @param[out] user_module_map A DAQModuleMap that will contain pointers to
   * all of the DAQModule instances, indexed by name
   * @param[out] command_order_map A map relating commands to an ordering of
   * DAQModules (by name)
   *
   * Brief This method is responsible for instantiating and linking all of the
   * Queue and DAQModule instances in a DAQ Application. Additionally, any
   * requirements on command order for DAQModules should be defined here.
   */
  virtual void ConstructGraph(QueueMap& queue_map,
                              DAQModuleMap& daq_module_map,
                              CommandOrderMap& command_order_map) = 0;
};
} // namespace appframework

#endif // APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_CORE_MODULELIST_HH_
