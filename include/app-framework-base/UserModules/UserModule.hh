#ifndef app_framework_base_UserModules_UserModule_hh
#define app_framework_base_UserModules_UserModule_hh

#include "app-framework-base/Core/StateMachine.hh"

namespace appframework {
class UserModule {
   public:
    virtual void execute_transition(TransitionName cmd) = 0;
};
}  // namespace appframework

#endif  // app_framework_base_UserModules_UserModule_hh