#ifndef app_framework_core_Services_CommandFacility_hh
#define app_framework_core_Services_CommandFacility_hh

#include <list>
#include <memory>

namespace appframework {
class DAQProcess;  // forward declaration

class CommandFacility {
   public:
    static std::unique_ptr<CommandFacility> const& handle() {
        if (!handle_) handle_.reset(new CommandFacility());
        return handle_;
    }
    static void setup(std::list<std::string>) {}
    virtual int listen(DAQProcess* /*process*/) { return 0; }

   protected:
    CommandFacility() {}

   private:
    static std::unique_ptr<CommandFacility> handle_;
};
}  // namespace appframework

#endif  // app_framework_core_Services_CommandFacility_hh