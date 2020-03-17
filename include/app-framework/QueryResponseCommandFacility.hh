#ifndef APP_FRAMEWORK_QUERYRESPONSECOMMANDFACILITY_HH
#define APP_FRAMEWORK_QUERYRESPONSECOMMANDFACILITY_HH

#include "app-framework-base/Services/CommandFacility.hh"
#include "app-framework-base/Core/StateMachine.hh"
#include "app-framework/DAQProcess.hh"

namespace appframework {
class QueryResponseCommandFacility : public CommandFacility {
public:
	virtual int listen(DAQProcess* theProcess) override;
private:
	TransitionName stringToTransitionName(std::string string);
};
}

#endif // APP_FRAMEWORK_QUERYRESPONSECOMMANDFACILITY_HH