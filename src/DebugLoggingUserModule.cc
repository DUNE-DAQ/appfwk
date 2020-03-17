#include "app-framework/DebugLoggingUserModule.hh"

#include "app-framework-base/Services/Logger.hh"
#include "app-framework-base/UserModules/UserModule.hh"

#include <iostream>

namespace appframework {
void DebugLoggingUserModule::execute_transition(TransitionName cmd) { std::cout << "Executing transition: " << transitionNameToString(cmd) << std::endl; }
std::string DebugLoggingUserModule::transitionNameToString(TransitionName cmd) {
    switch (cmd) {
        case TransitionName::Start:
            return "Start";
        case TransitionName::Stop:
            return "Stop";
    }
    return "UnknownTransition";
}
}  // namespace appframework