/**
 * @file DAQProcess class implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#include "app-framework/DAQProcess.hh"

#include "app-framework-base/Services/CommandFacility.hh"
#include "app-framework-base/Services/ConfigurationManager.hh"
#include "app-framework-base/Services/Logger.hh"
#include "app-framework-base/Services/ServiceManager.hh"

namespace appframework {
ServiceManager* ServiceManager::handle_ = nullptr;
ConfigurationManager* ConfigurationManager::handle_ = nullptr;

DAQProcess::DAQProcess(std::list<std::string> args) {
    Logger::setup(args);
    CommandFacility::setup(args);
    ConfigurationManager::setup(args);
    ServiceManager::setup(args);
}

void DAQProcess::register_modules(std::unique_ptr<ModuleList> const& ml) {
    ml->ConstructGraph(bufferMap_, userModuleMap_, commandOrderMap_);
}

void DAQProcess::execute_command(std::string cmd) {
    if (commandOrderMap_.count(cmd)) {
        for (auto& moduleName : commandOrderMap_[cmd]) {
            if (userModuleMap_.count(moduleName)) {
                userModuleMap_[moduleName]->execute_command(cmd);
            }
        }
    } else {
        for (auto const& um : userModuleMap_) {
            um.second->execute_command(cmd);
        }
    }
}

int DAQProcess::listen() { return CommandFacility::handle()->listen(this); }
}  // namespace appframework