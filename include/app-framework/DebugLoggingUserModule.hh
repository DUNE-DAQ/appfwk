#ifndef APP_FRAMEWORK_DEBUGLOGGINGUSERMODULE_HH
#define APP_FRAMEWORK_DEBUGLOGGINGUSERMODULE_HH

#include "app-framework-base/UserModules/UserModule.hh"

#include <string>

namespace appframework {
class DebugLoggingUserModule : public UserModule {
   public:
    void execute_transition(TransitionName cmd) override;

   private:
    std::string transitionNameToString(TransitionName cmd);
};
}  // namespace appframework

#endif  // APP_FRAMEWORK_DEBUGLOGGINGUSERMODULE_HH