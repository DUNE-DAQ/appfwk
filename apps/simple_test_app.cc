#include "app-framework/DAQProcess.hh"
#include "app-framework/DebugLoggingUserModule.hh"
#include "app-framework/QueryResponseCommandFacility.hh"

namespace appframework {
std::unique_ptr<CommandFacility> CommandFacility::handle_ = std::unique_ptr<CommandFacility>(new QueryResponseCommandFacility());

class simple_test_app_ModuleList : public ModuleList {
    // Inherited via ModuleList
    virtual void ConstructGraph(BufferMap& buffer_map, UserModuleMap& user_module_map, TransitionOrderMap& transition_order_map) override {
        user_module_map["debugLogger"].reset(new DebugLoggingUserModule());
    }
};
}  // namespace appframework

int main(int argc, char* argv[]) {
    std::list<std::string> args;
    for (int ii = 1; ii < argc; ++ii) {
        args.push_back(std::string(argv[ii]));
    }

    appframework::DAQProcess theDAQProcess(args);

    auto ml = std::unique_ptr<appframework::ModuleList>(new appframework::simple_test_app_ModuleList());
    theDAQProcess.register_modules(ml);

    return theDAQProcess.listen();
}