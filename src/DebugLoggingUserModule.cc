/**
 * @file DebugLoggingUserModule class implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#include "app-framework/DebugLoggingUserModule.hh"

#include "app-framework-base/Services/Logger.hh"
#include "app-framework-base/UserModules/UserModule.hh"

#include <iostream>

namespace appframework {
std::string DebugLoggingUserModule::execute_transition(TransitionName cmd) {
    std::cout << "Executing transition: " << transitionNameToString(cmd) << std::endl;
    return "Success";
}
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