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

#include "cetlib/BasicPluginFactory.h"
#include "cetlib/compiler_macros.h"
#include <memory>
#include <string>

#ifndef EXTERN_C_FUNC_DECLARE_START
#define EXTERN_C_FUNC_DECLARE_START                                            \
  extern "C"                                                                   \
  {
#endif

#define DEFINE_DUNE_SERVICE(klass)                                             \
  EXTERN_C_FUNC_DECLARE_START                                                  \
  std::unique_ptr<appframework::Service> make()                                \
  {                                                                            \
    return std::unique_ptr<appframework::Service>(new klass());                \
  }                                                                            \
  }

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

std::unique_ptr<Service>
makeService(std::string const& service_name)
{
  static cet::BasicPluginFactory bpf("duneService", "make");

  return bpf.makePlugin<std::unique_ptr<Service>>(service_name);
}

} // namespace appframework

#endif // APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_SERVICES_SERVICE_HH_
