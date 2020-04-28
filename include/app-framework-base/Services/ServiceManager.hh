/**
 * @file ServiceManager interface definition
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_SERVICES_SERVICEMANAGER_HH_
#define APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_SERVICES_SERVICEMANAGER_HH_

#include "app-framework-base/Services/Service.hh"

#include <list>
#include <map>
#include <memory>
#include <string>
#include <utility>

namespace appframework {
/**
 * @brief The ServiceManager class manages all non-essential Service instances
 * which may be needed by a particular DAQ Application
 */
class ServiceManager
{
public:
  /**
   * @brief Setup the ServiceManager
   * @param args Command-line arguments, may contain list of services to load
   * and parameters for each
   */
  static void setup(std::list<std::string> /*args*/) {}

  /**
   * @brief Get a handle to the ServiceManager (Singleton pattern)
   * @return Reference to the ServiceManager
   */
  static ServiceManager& handle()
  {
    if (!handle_)
      handle_.reset(new ServiceManager());
    return *handle_;
  }

  /**
   * @brief Get a pointer to the named Service instance
   * @param service_name Name of the Service
   * @return Pointer to the named service
   */
  std::unique_ptr<Service> const& getService(std::string service_name)
  {
    return service_map_[service_name];
  }
  /**
   * @brief Add a Service instance to the ServiceManager
   * @param name Name used to identify this Service instance
   * @param service Service instance
   */
  void register_service(std::string name, std::unique_ptr<Service>&& service)
  {
    service_map_[name] = std::move(service);
  }

private:
  /**
   * @brief Singleton pattern, private parameterless constructor
   */
  ServiceManager() {}

  std::map<std::string, std::unique_ptr<Service>>
    service_map_; ///< Registers Service instances
  static std::unique_ptr<ServiceManager>
    handle_; ///< Singleton pattern, static handle
};
} // namespace appframework

#endif // APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_SERVICES_SERVICEMANAGER_HH_
