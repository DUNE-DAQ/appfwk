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

std::map<std::string, std::shared_ptr<appfwk::DAQModule>> module_map;
std::map<std::string, std::vector<std::string>> modules_by_type;

std::string red = "";    // NOLINT
std::string green = "";  // NOLINT
std::string yellow = ""; // NOLINT
std::string blue = "";   // NOLINT
std::string clear = "";  // NOLINT

struct ErrorReport
{
  enum Severity
  {
    Error,
    Warning,
    Info,
    Ignored
  };
  Severity severity;
  std::string app;
  std::string module;
  std::string command;
  std::string message;

  std::string severity_color()
  {
    switch (severity) {
      case ErrorReport::Severity::Error:
        return red;
      case ErrorReport::Severity::Warning:
        return yellow;
      case ErrorReport::Severity::Info:
        return blue;
      case ErrorReport::Severity::Ignored:
        return green;
    }
    return clear;
  }
  std::string severity_string()
  {
    switch (severity) {
      case ErrorReport::Severity::Error:
        return "Error";
      case ErrorReport::Severity::Warning:
        return "Warning";
      case ErrorReport::Severity::Info:
        return "Info";
      case ErrorReport::Severity::Ignored:
        return "Debug";
    }
    return "UNKNOWN";
  }
};
std::vector<ErrorReport> validation_errors;

void
check_mod_has_cmd(const std::string& app,
                  const std::string& cmd,
                  const std::string& mod_class,
                  bool is_optional,
                  const std::string& mod_id = "")
{
  if (!modules_by_type.count(mod_class) || modules_by_type[mod_class].size() == 0) {
    if (is_optional) {
      validation_errors.emplace_back(ErrorReport::Severity::Ignored,
                                     app,
                                     mod_class,
                                     cmd,
                                     "No modules of class " + mod_class + " in application (optional step)");
      return;
    }
    if (mod_id == "") {
      validation_errors.emplace_back(
        ErrorReport::Severity::Warning, app, mod_class, cmd, "No modules of class " + mod_class + " in application!");
      ers::warning(appfwk::ActionPlanValidationFailed(
        ERS_HERE, cmd, mod_class, "No modules of class " + mod_class + " in application!"));
      return;
    } else {
      validation_errors.emplace_back(
        ErrorReport::Severity::Error, app, mod_class, cmd, "No modules of class " + mod_class + " in application!");
      ers::error(appfwk::ActionPlanValidationFailed(
        ERS_HERE, cmd, mod_class, "No modules of class " + mod_class + " in application!"));
    }
  }

  auto module_test = module_map[modules_by_type[mod_class][0]];
  if (mod_id != "") {
    bool match = false;
    for (auto& mod_name : modules_by_type[mod_class]) {
      if (mod_id == mod_name) {
        module_test = module_map[mod_name];
        match = true;
        break;
      }
    }
    if (!match && !is_optional) {
      validation_errors.emplace_back(
        ErrorReport::Severity::Error, app, mod_class, cmd, "No module with id " + mod_id + " found.");
      ers::error(
        appfwk::ActionPlanValidationFailed(ERS_HERE, cmd, mod_class, "No module with id " + mod_id + " found."));
    }
  }

  if (!module_test->has_command(cmd)) {
    validation_errors.emplace_back(
      ErrorReport::Severity::Error, app, mod_class, cmd, "Module does not have command " + cmd + " registered.");
    ers::error(appfwk::ActionPlanValidationFailed(
      ERS_HERE, cmd, mod_class, "Module does not have command " + cmd + " registered."));
  }
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

  for (auto& app : apps) {
    module_map.clear();
    modules_by_type.clear();

    TLOG() << app->UID() << ": Initializing ConfigurationManager to check for duplicate ActionPlans";
    auto cfgMgr = std::make_shared<appfwk::ConfigurationManager>(dbfile, app->UID(), sessionName);
    cfgMgr->initialize();

    // Check module matching
    auto modules = cfgMgr->modules();

    for (const auto mod : modules) {
      TLOG_DEBUG(0) << "construct: " << mod->class_name() << " : " << mod->UID();
      auto mptr = appfwk::make_module(mod->class_name(), mod->UID());
      module_map.emplace(mod->UID(), mptr);

      if (!modules_by_type.count(mod->class_name())) {
        modules_by_type[mod->class_name()] = std::vector<std::string>();
      }
      modules_by_type[mod->class_name()].emplace_back(mod->UID());
    }

    for (auto& plan_pair : cfgMgr->action_plans()) {
      auto cmd = plan_pair.first;
      TLOG() << app->UID() << ": Checking action plan " << cmd;
      std::map<std::string, std::set<std::string>> modules_with_cmd;
      for (const auto& [mod_type, module_list] : modules_by_type) {
        if (module_list.size() > 0 && module_map[module_list[0]]->has_command(cmd)) {
          modules_with_cmd[mod_type] = std::set<std::string>(module_list.begin(), module_list.end());
        }
      }

      for (auto& step : plan_pair.second->get_steps()) {
        auto byType = step->cast<confmodel::DaqModulesGroupByType>();
        auto byMod = step->cast<confmodel::DaqModulesGroupById>();
        if (byType != nullptr) {
          for (auto& mod_type : byType->get_modules()) {
            check_mod_has_cmd(app->UID(), cmd, mod_type, byType->get_optional());
            modules_with_cmd.erase(mod_type);
          }
        } else if (byMod != nullptr) {
          for (auto& mod : byMod->get_modules()) {
            check_mod_has_cmd(app->UID(), cmd, mod->class_name(), byMod->get_optional(), mod->UID());
            modules_with_cmd[mod->class_name()].erase(mod->UID());
          }
        } else {
          validation_errors.emplace_back(
            ErrorReport::Severity::Error, app->UID(), "N/A", cmd, "Invalid subclass of DaqModulesGroup encountered!");
          ers::error(
            appfwk::ActionPlanValidationFailed(ERS_HERE, cmd, "", "Invalid subclass of DaqModulesGroup encountered!"));
        }
      }

      for (const auto& [mod_type, module_list] : modules_with_cmd) {
        for (auto& mod : module_list) {
          validation_errors.emplace_back(ErrorReport::Severity::Error,
                                         app->UID(),
                                         mod_type,
                                         cmd,
                                         "ActionPlan is defined, module has command, but module " + mod +
                                           " is not in any steps");
          ers::error(appfwk::ActionPlanValidationFailed(
            ERS_HERE, cmd, mod, "ActionPlan is defined, module has command, but module is not in any steps"));
        }
      }
    }
  }

  std::cout << std::endl << std::endl << "Summary:" << std::endl; // NOLINT

  if (validation_errors.size() > 0) {
    size_t longest_app = 11;     // Application heading
    size_t longest_command = 7;  // Command heading
    size_t longest_module = 6;   // Module heading
    size_t longest_severity = 8; // Severity heading

    for (auto& report : validation_errors) {
      if (report.app.size() > longest_app)
        longest_app = report.app.size();
      if (report.command.size() > longest_command)
        longest_command = report.command.size();
      if (report.module.size() > longest_module)
        longest_module = report.module.size();
    }

    std::string app_heading_space(longest_app - 11 + minimum_space, ' ');
    std::string command_heading_space(longest_command - 7 + minimum_space, ' ');
    std::string module_heading_space(longest_module - 6 + minimum_space, ' ');
    std::string severity_heading_space(longest_severity - 8 + minimum_space, ' ');
    // NOLINTNEXTLINE
    std::cout << "Application" << app_heading_space << "Command" << command_heading_space << "Module"
              << module_heading_space << "Severity" << severity_heading_space << "Message" << std::endl;

    for (auto& report : validation_errors) {

      std::cout << report.severity_color(); // NOLINT
      std::string app_space(longest_app - report.app.size() + minimum_space, ' ');
      std::cout << report.app << app_space; // NOLINT
      std::string command_space(longest_command - report.command.size() + minimum_space, ' ');
      std::cout << report.command << command_space; // NOLINT
      std::string module_space(longest_module - report.module.size() + minimum_space, ' ');
      std::cout << report.module << module_space; // NOLINT
      std::string severity_space(longest_severity - report.severity_string().size() + minimum_space, ' ');
      std::cout << report.severity_string() << severity_space; // NOLINT
      std::cout << report.message << clear << std::endl;       // NOLINT
    }
  } else {
    std::cout << "No validation errors encountered!" << std::endl; // NOLINT
  }
} // NOLINT
