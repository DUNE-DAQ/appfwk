#ifndef app_framework_base_Services_ConfigurationManager_hh
#define app_framework_base_Services_ConfigurationManager_hh

namespace appframework {

class ConfigurationManager {
   public:
    static ConfigurationManager* handle() {
        if (!handle_) handle_ = new ConfigurationManager();
        return handle_;
    }
    static void setup(std::list<std::string>) {}

   protected:
    ConfigurationManager() {}

   private:
    static ConfigurationManager* handle_;
};
}  // namespace appframework

#endif  // app_framework_base_Services_ConfigurationManager_hh