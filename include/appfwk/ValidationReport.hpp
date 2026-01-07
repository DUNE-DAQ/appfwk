/**
 * @file ValidationReport.hpp Container for ActionPlan validation messages
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_VALIDATIONREPORT_HPP_
#define APPFWK_INCLUDE_APPFWK_VALIDATIONREPORT_HPP_

#include <string>

namespace dunedaq {


namespace appfwk {

class ValidationReport
{
public:
  enum class Severity
  {
    kFatal,
    kError,
    kWarning,
    kInfo,
    kIgnored
  };

  ValidationReport(Severity sev, std::string app, std::string module, std::string command, std::string message)
    : m_severity(sev)
    , m_app(app)
    , m_module(module)
    , m_command(command)
    , m_message(message)
  {
  }

  std::string severity_string()
  {
    switch (m_severity) {
      case Severity::kFatal:
        return "Fatal";
      case Severity::kError:
        return "Error";
      case Severity::kWarning:
        return "Warning";
      case Severity::kInfo:
        return "Info";
      case Severity::kIgnored:
        return "Debug";
    }
    return "UNKNOWN";
  }

  Severity get_severity() const { return m_severity; }
  std::string get_app() const { return m_app; }
  std::string get_module() const { return m_module; }
  std::string get_command() const { return m_command; }
  std::string get_message() const { return m_message; }

private:
  Severity m_severity;
  std::string m_app;
  std::string m_module;
  std::string m_command;
  std::string m_message;
};

} // namespace appfwk
} //namespace dunedaq

#endif // APPFWK_INCLUDE_APPFWK_VALIDATIONREPORT_HPP_
