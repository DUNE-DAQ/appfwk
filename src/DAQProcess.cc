#include <future>

#include "app-framework/DAQProcess.hh"
#include "app-framework-base/Services/Logger.hh"
#include "app-framework-base/Services/CommandFacility.hh"
#include "app-framework-base/Services/ConfigurationManager.hh"
#include "app-framework-base/Services/ServiceManager.hh"

appframework::DAQProcess::DAQProcess(std::list<std::string> args)
	: myState_(StateMachineState::Initial)
{
	Logger::handle().setup(args);
	CommandFacility::handle().setup(args);
	ConfigurationManager::handle().setup(args);
	ServiceManager::handle().setup(args);
	myState_ = StateMachineState::Booted;
}

void appframework::DAQProcess::register_modules(std::unique_ptr<ModuleList> ml)
{
	ml->ConstructGraph(bufferMap_, userModuleMap_, transitionOrderMap_);
	myState_ = StateMachineState::ModulesRegistered;
}

void appframework::DAQProcess::execute_transition(TransitionName cmd)
{
	if (transitionOrderMap_.count(cmd)) {
		for (auto& moduleName : transitionOrderMap_[cmd]) {
			if (userModuleMap_.count(moduleName)) {
				userModuleMap_[moduleName]->execute_transition(cmd);
			}
		}
	}
	else {
		for (auto const& um : userModuleMap_) {
			um.second->execute_transition(cmd);
		}
	}
}

std::future<int> appframework::DAQProcess::listen()
{
	std::future<int> fut = CommandFacility::handle().listen();
	return fut;
}
