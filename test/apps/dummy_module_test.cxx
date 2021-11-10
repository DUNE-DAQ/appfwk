/**
 * @file dummy_module_example.cxx
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/DAQModule.hpp"
#include "appfwk/QueueRegistry.hpp"
#include "appfwk/app/Nljs.hpp"

#include "logging/Logging.hpp" // NOLINT

#include <iostream>
#include <map>
#include <memory>
#include <string>

using namespace dunedaq::appfwk;

int
main()
{
  TLOG() << "Creating Module instances...";
  std::shared_ptr<DAQModule> dummy_module = make_module("DummyModule", "dummy");

  std::map<std::string, QueueConfig> queue_map;
  QueueRegistry::get().configure(queue_map);

  // Init
  TLOG() << "Calling init on modules...";
  app::ModInit dummy_init_data;
  nlohmann::json dummy_init_json;
  to_json(dummy_init_json, dummy_init_data);
  dummy_module->init(dummy_init_json);

  TLOG() << "Calling stuff on module...";
  dummy_module->execute_command("stuff");

  TLOG() << "Calling bad_stuff on module...";
  try {
    dummy_module->execute_command("bad_stuff");
    TLOG() << "Should have thrown exception";
  } catch (GeneralDAQModuleIssue&) {
    TLOG() << "Exception thrown as expected";
  }

  TLOG() << "Test complete";
}
