#include "app-framework/DAQProcess.hh"

#include "app-framework-base/Services/CommandFacility.hh"
#include "app-framework-base/Services/ConfigurationManager.hh"
#include "app-framework-base/Services/Logger.hh"
#include "app-framework-base/Services/ServiceManager.hh"

namespace appframework {
ServiceManager* ServiceManager::handle_ = nullptr;
ConfigurationManager* ConfigurationManager::handle_ = nullptr;

DAQProcess::DAQProcess(std::list<std::string> args) : myState_(StateMachineState::Initial) {
    Logger::setup(args);
    CommandFacility::setup(args);
    ConfigurationManager::setup(args);
    ServiceManager::setup(args);
    myState_ = StateMachineState::Booted;
}

void DAQProcess::register_modules(std::unique_ptr<ModuleList> const& ml) {
    ml->ConstructGraph(bufferMap_, userModuleMap_, transitionOrderMap_);
    myState_ = StateMachineState::ModulesRegistered;
}

void DAQProcess::execute_transition(TransitionName cmd) {
    if (transitionOrderMap_.count(cmd)) {
        for (auto& moduleName : transitionOrderMap_[cmd]) {
            if (userModuleMap_.count(moduleName)) {
                userModuleMap_[moduleName]->execute_transition(cmd);
            }
        }
    } else {
        for (auto const& um : userModuleMap_) {
            um.second->execute_transition(cmd);
        }
    }
}

int DAQProcess::listen() { return CommandFacility::handle()->listen(this); }
}  // namespace appframework