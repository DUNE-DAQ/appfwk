/**
 * @file ConfigurationManager service interface definition
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#ifndef app_framework_base_Services_ConfigurationManager_hh
#define app_framework_base_Services_ConfigurationManager_hh

namespace appframework {
/**
 * @brief The ConfigurationManager is responsible for receiving configurations from CCM and making them available to
 * UserModules
 */
class ConfigurationManager {
   public:
    /**
     * @brief Singleton pattern, get a handle to the ConfigurationManager
     * @return Pointer to the ConfigurationManager
     */
    static ConfigurationManager* handle() {
        if (!handle_) handle_ = new ConfigurationManager();
        return handle_;
    }
    /**
     * @brief Perform basic setup actions needed by ConfigurationManager, using the provided command-line paramters
     * @param args Command-line arguments used to setup ConfigurationManager
     */
    static void setup(std::list<std::string> /*args*/) {}

   protected:
    ConfigurationManager() {}  ///< Singleton pattern, protected parameterless constructor

   private:
    static ConfigurationManager* handle_;  ///< Singleton pattern, static handle_ member
};
}  // namespace appframework

#endif  // app_framework_base_Services_ConfigurationManager_hh