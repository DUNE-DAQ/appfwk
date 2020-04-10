/**
 * @file Service interface definition
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#ifndef app_framework_base_Services_Service_hh
#define app_framework_base_Services_Service_hh

namespace appframework {
/**
 * @brief The Service class represents a static entity which may be loaded into the ServiceManager for
 * use by multiple UserModules and/or Buffers
 */
class Service {
   public:
    /**
     * @brief Configure the Service instance with command-line-like arguments
     * @param args Arguments used to configure the Service
     */
    virtual void setup(std::list<std::string> args) = 0;
};
}  // namespace appframework

#endif  // app_framework_base_Services_Service_hh