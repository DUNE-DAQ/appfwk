/**
 * @file Service interface definition
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_SERVICES_SERVICE_HH_
#define APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_SERVICES_SERVICE_HH_

#include <list>
#include <string>

namespace appframework {
/**
 * @brief The Service class represents a static entity which may be loaded into
 * the ServiceManager for use by multiple DAQModules and/or Queues
 */
class Service
{
public:
  /**
   * @brief Configure the Service instance with command-line-like arguments
   * @param args Arguments used to configure the Service
   */
  virtual void setup(std::list<std::string> args) = 0;
};
} // namespace appframework

#endif // APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_SERVICES_SERVICE_HH_
