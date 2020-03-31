#ifndef app_framework_base_Core_ModuleList_hh
#define app_framework_base_Core_ModuleList_hh

#include "app-framework-base/Buffers/Buffer.hh"
#include "app-framework-base/Core/StateMachine.hh"
#include "app-framework-base/UserModules/UserModule.hh"

#include <list>
#include <map>
#include <memory>
#include <string>

namespace appframework {
typedef std::map<std::string, std::unique_ptr<UserModule>> UserModuleMap;
typedef std::map<std::string, std::unique_ptr<Buffer>> BufferMap;
typedef std::map<TransitionName, std::list<std::string>> TransitionOrderMap;

class ModuleList {
   public:
    /**
     * @brief Construct the graph of UserModules and Buffers.
     */
    virtual void ConstructGraph(BufferMap& buffer_map, UserModuleMap& user_module_map, TransitionOrderMap& transition_order_map) = 0;
};
}  // namespace appframework

#endif  // app_framework_base_Core_ModuleList_hh
