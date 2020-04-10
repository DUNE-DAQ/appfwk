/**
 * @file ModuleList class interface
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#ifndef app_framework_base_Core_ModuleList_hh
#define app_framework_base_Core_ModuleList_hh

#include "app-framework-base/Buffers/Buffer.hh"
#include "app-framework-base/UserModules/UserModule.hh"

#include <list>
#include <map>
#include <memory>
#include <string>

namespace appframework {
typedef std::map<std::string, std::unique_ptr<UserModule>> UserModuleMap; ///< UserModules indexed by name
typedef std::map<std::string, std::unique_ptr<Buffer>> BufferMap; ///< Buffers indexed by name
typedef std::map<std::string, std::list<std::string>> CommandOrderMap; ///< Defined UserModule orders for commands

/**
 * @brief The ModuleList class is the representation of a UserModule and Buffer graph
 *
 * The ConstructGraph method is responsible for instantiating all Buffers and UserModules for a DAQ Application,
 * as well as linking them together. UserModules define their Buffer endpoints by requiring Buffer instances
 */
class ModuleList {
   public:
    /**
     * @brief Construct the graph of UserModules and Buffers.
     * @param[out] buffer_map A BufferMap that will contain pointers to all of the Buffer instances, indexed by name
     * @param[out] user_module_map A UserModuleMap that will contain pointers to  all of the UserModule instances, indexed by name
     * @param[out] command_order_map A map relating commands to an ordering of UserModules (by name)
     *
     * Brief This method is responsible for instantiating and linking all of the Buffer and UserModule instances in a DAQ Application.
     * Additionally, any requirements on command order for UserModules should be defined here.
     */
    virtual void ConstructGraph(BufferMap& buffer_map, UserModuleMap& user_module_map, CommandOrderMap& command_order_map) = 0;
};
}  // namespace appframework

#endif  // app_framework_base_Core_ModuleList_hh
