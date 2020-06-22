/**
 * @file Logger.hpp Logger interface definition
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_LOGGER_HPP_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_LOGGER_HPP_

#include <string>
#include <vector>

namespace dunedaq::appfwk {
/**
 * @brief The Logger class defines the interface necessary to configure central
 * logging within a DAQ Application.
 */
class Logger
{
public:
  /**
   * @brief Setup the Logger service
   * @param args Command-line arguments used to setup the Logger
   */
  static void setup(const std::vector<std::string>& /*args*/) {}
};
} // namespace dunedaq::appfwk

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_LOGGER_HPP_
