/**
 * @file Logger interface definition
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#ifndef app_framework_base_Services_Logger_hh
#define app_framework_base_Services_Logger_hh

#include <vector>
#include <string>

namespace appframework {
/**
 * @brief The Logger class defines the interface necessary to configure central logging within a DAQ Application.
 */
class Logger {
   public:
    /**
     * @brief Setup the Logger service
     * @param args Command-line arguments used to setup the Logger
     */
    static void setup(std::vector<std::string> /*args*/) {}
};
}  // namespace appframework

#endif  // app_framework_base_Services_Logger_hh