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
#include <string>

using namespace dunedaq;

std::map<std::string, std::shared_ptr<appfwk::DAQModule>> module_map;
std::map<std::string, std::vector<std::string>> modules_by_type;
void
check_mod_has_cmd(const std::string& cmd,
                  const std::string& mod_class,
                  bool is_optional,
                  const std::string& mod_id = "")
{
  if (!modules_by_type.count(mod_class) || modules_by_type[mod_class].size() == 0) {
    if (is_optional)
      return;
    if (mod_id == "") {
      ers::warning(appfwk::ActionPlanValidationFailed(ERS_HERE, cmd, mod_class, "Module does not exist"));
      return;
    } else {
      ers::error(appfwk::ActionPlanValidationFailed(ERS_HERE, cmd, mod_class, "Module does not exist"));
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
      ers::error(
        appfwk::ActionPlanValidationFailed(ERS_HERE, cmd, mod_class, "No module with id " + mod_id + " found."));
    }
  }

  if (!module_test->has_command(cmd)) {
    ers::error(appfwk::ActionPlanValidationFailed(ERS_HERE, cmd, mod_class, "Module does not have method " + cmd));
  }
}
int
main(int argc, char* argv[])
{
  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " <session> <database-file>\n";
    return 0;
  }

  std::string sessionName(argv[1]);
  std::string dbfile(argv[2]);
  if (dbfile.find(":") == std::string::npos) {
    dbfile = "oksconflibs:" + dbfile;
  }

  logging::Logging::setup("test", "validate_plans");

  conffwk::Configuration* confdb;
  try {
    confdb = new conffwk::Configuration(dbfile);
  } catch (conffwk::Generic& exc) {
    std::cout << "Failed to load OKS database: " << exc << std::endl;
    return 0;
  }

  auto session = confdb->get<confmodel::Session>(sessionName);
  if (session == nullptr) {
    std::cout << "Failed to get Session " << sessionName << " from database\n";
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
            check_mod_has_cmd(cmd, mod_type, byType->get_optional());
            modules_with_cmd.erase(mod_type);
          }
        } else if (byMod != nullptr) {
          for (auto& mod : byMod->get_modules()) {
            check_mod_has_cmd(cmd, mod->class_name(), byMod->get_optional(), mod->UID());
            modules_with_cmd[mod->class_name()].erase(mod->UID());
          }
        } else {
          ers::error(
            appfwk::ActionPlanValidationFailed(ERS_HERE, cmd, "", "Invalid subclass of DaqModulesGroup encountered!"));
        }
      }

      for (const auto& [mod_type, module_list] : modules_with_cmd) {
        for (auto& mod : module_list) {
          ers::error(appfwk::ActionPlanValidationFailed(
            ERS_HERE, cmd, mod, "ActionPlan is defined, module has command, but module is not in any steps"));
        }
      }
    }
  }
}
