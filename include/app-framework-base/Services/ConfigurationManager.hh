/**
 * @file ConfigurationManager service interface definition
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_SERVICES_CONFIGURATIONMANAGER_HH_
#define APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_SERVICES_CONFIGURATIONMANAGER_HH_

#include <list>
#include <memory>
#include <string>

namespace appframework {
/**
 * @brief The ConfigurationManager is responsible for receiving configurations
 * from CCM and making them available to DAQModules
 */
class ConfigurationManager
{
public:
  /**
   * @brief Singleton pattern, get a handle to the ConfigurationManager
   * @return Reference to the ConfigurationManager
   */
  static ConfigurationManager& handle()
  {
    if (!handle_)
      handle_.reset(new ConfigurationManager());
    return *handle_;
  }
  /**
   * @brief Perform basic setup actions needed by ConfigurationManager, using
   * the provided command-line paramters
   * @param args Command-line arguments used to setup ConfigurationManager
   */
  static void setup(std::list<std::string> /*args*/) {}

protected:
  ConfigurationManager() {
  } ///< Singleton pattern, protected parameterless constructor

private:
  static std::unique_ptr<ConfigurationManager>
    handle_; ///< Singleton pattern, static handle_ member
};
} // namespace appframework

#endif // APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_SERVICES_CONFIGURATIONMANAGER_HH_
