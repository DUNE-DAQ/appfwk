/**
 * @file check_action_plans.cxx
 *
 * Run the DAQModuleManager ActionPlan validation on each app in the given session
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "logging/Logging.hpp"

#include "DAQModuleManager.hpp"
#include "appfwk/ConfigurationManager.hpp"
#include "appfwk/DAQModule.hpp"
#include "appfwk/ValidationReport.hpp"

#include "conffwk/Configuration.hpp"
#include "confmodel/DaqModulesGroup.hpp"
#include "confmodel/DaqModulesGroupById.hpp"
#include "confmodel/DaqModulesGroupByType.hpp"
#include "confmodel/Session.hpp"
#include "opmonlib/TestOpMonManager.hpp"

#include <iostream>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

using namespace dunedaq;

std::string red = "";    // NOLINT
std::string green = "";  // NOLINT
std::string yellow = ""; // NOLINT
std::string blue = "";   // NOLINT
std::string clear = "";  // NOLINT

std::string
severity_color(appfwk::ValidationReport report)
{
  switch (report.get_severity()) {
    case appfwk::ValidationReport::Severity::kFatal:
      return red;
    case appfwk::ValidationReport::Severity::kError:
      return red;
    case appfwk::ValidationReport::Severity::kWarning:
      return yellow;
    case appfwk::ValidationReport::Severity::kInfo:
      return blue;
    case appfwk::ValidationReport::Severity::kIgnored:
      return green;
  }
  return clear;
}

int
main(int argc, char* argv[])
{
  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " <session> <database-file> [spacing=2] [nocolor]\n"; // NOLINT
    return 0;
  }

  std::string sessionName(argv[1]); // NOLINT
  std::string dbfile(argv[2]);      // NOLINT
  size_t minimum_space = 2;         // Minimum space between columns (must be >= 1)
  if (argc >= 4) {
    minimum_space = std::atoi(argv[3]); // NOLINT
    if (minimum_space < 1) {
      minimum_space = 1;
    }
    if (minimum_space > 10) {
      minimum_space = 10;
    }
  }

  if (argc < 5) {
    red = "\033[31m";
    green = "\033[32m";
    yellow = "\033[33m";
    blue = "\033[36m";
    clear = "\033[0m";
  }

  if (dbfile.find(":") == std::string::npos) {
    dbfile = "oksconflibs:" + dbfile;
  }

  logging::Logging::setup("test", "validate_plans");

  conffwk::Configuration* confdb = nullptr;
  try {
    confdb = new conffwk::Configuration(dbfile);
  } catch (conffwk::Generic& exc) {
    std::cout << "Failed to load OKS database: " << exc << std::endl; // NOLINT
    return 0;
  }

  auto session = confdb->get<confmodel::Session>(sessionName);
  if (session == nullptr) {
    std::cout << "Failed to get Session " << sessionName << " from database\n"; // NOLINT
    return 0;
  }

  auto apps = session->enabled_applications();

  std::vector<appfwk::ValidationReport> validation_errors;

  for (auto& app : apps) {

    TLOG() << app->UID() << ": Initializing ConfigurationManager to check for duplicate ActionPlans";
    auto cfgMgr = std::make_shared<appfwk::ConfigurationManager>(dbfile, app->UID(), sessionName);
    auto cfgMgr_reports = cfgMgr->initialize(false);
    validation_errors.insert(validation_errors.end(),
                             std::make_move_iterator(cfgMgr_reports.begin()),
                             std::make_move_iterator(cfgMgr_reports.end()));

    // Check module matching
    TLOG() << app->UID() << ": Constructing modules so they can register their commands";
    auto modules = cfgMgr->get_modules();
    appfwk::DAQModuleManager mmgr(sessionName);
    mmgr.set_config_mgr(cfgMgr);
    mmgr.construct_modules(modules);

    
    TLOG() << app->UID() << ": Validating Action Plans";
    auto ap_reports = mmgr.validate_action_plans(false);
    validation_errors.insert(validation_errors.end(),
                             std::make_move_iterator(ap_reports.begin()), std::make_move_iterator(ap_reports.end()));
  }

  std::cout << std::endl << std::endl << "Summary:" << std::endl; // NOLINT

  if (validation_errors.size() > 0) {
    size_t longest_app = 11;     // Application heading
    size_t longest_command = 7;  // Command heading
    size_t longest_module = 6;   // Module heading
    size_t longest_severity = 8; // Severity heading

    for (auto& report : validation_errors) {
      if (report.get_app().size() > longest_app)
        longest_app = report.get_app().size();
      if (report.get_command().size() > longest_command)
        longest_command = report.get_command().size();
      if (report.get_module().size() > longest_module)
        longest_module = report.get_module().size();
    }

    std::string app_heading_space(longest_app - 11 + minimum_space, ' ');
    std::string command_heading_space(longest_command - 7 + minimum_space, ' ');
    std::string module_heading_space(longest_module - 6 + minimum_space, ' ');
    std::string severity_heading_space(longest_severity - 8 + minimum_space, ' ');
    // NOLINTNEXTLINE
    std::cout << "Application" << app_heading_space << "Command" << command_heading_space << "Module"
              << module_heading_space << "Severity" << severity_heading_space << "Message" << std::endl;

    for (auto& report : validation_errors) {

      std::cout << severity_color(report); // NOLINT
      std::string app_space(longest_app - report.get_app().size() + minimum_space, ' ');
      std::cout << report.get_app() << app_space; // NOLINT
      std::string command_space(longest_command - report.get_command().size() + minimum_space, ' ');
      std::cout << report.get_command() << command_space; // NOLINT
      std::string module_space(longest_module - report.get_module().size() + minimum_space, ' ');
      std::cout << report.get_module() << module_space; // NOLINT
      std::string severity_space(longest_severity - report.severity_string().size() + minimum_space, ' ');
      std::cout << report.severity_string() << severity_space; // NOLINT
      std::cout << report.get_message() << clear << std::endl;       // NOLINT
    }
  } else {
    std::cout << "No validation errors encountered!" << std::endl; // NOLINT
  }
} // NOLINT
