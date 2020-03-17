#ifndef app_framework_DAQProcess_hh
#define app_framework_DAQProcess_hh

#include "app-framework-base/Buffers/Buffer.hh"
#include "app-framework-base/Core/ModuleList.hh"
#include "app-framework-base/Core/StateMachine.hh"
#include "app-framework-base/UserModules/UserModule.hh"

#include <map>
#include <string>

namespace appframework {
class DAQProcess {
   public:
    DAQProcess(std::list<std::string> args);
    void register_modules(std::unique_ptr<ModuleList> const& ml);
    void execute_transition(TransitionName cmd);
    int listen();

   private:
    StateMachineState myState_;

    BufferMap bufferMap_;                    ///< String alias for each Buffer
    UserModuleMap userModuleMap_;            ///< String alias for each UserModule
    TransitionOrderMap transitionOrderMap_;  ///< Order UserModule transitions by alias
};
}  // namespace appframework

#endif  // app_framework_DAQProcess_hh