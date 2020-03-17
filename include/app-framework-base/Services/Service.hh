#ifndef app_framework_base_Services_Service_hh
#define app_framework_base_Services_Service_hh

namespace appframework {
class Service {
   public:
    virtual void setup(std::list<std::string> args) = 0;

    Service() {}
};
}  // namespace appframework

#endif  // app_framework_base_Services_Service_hh