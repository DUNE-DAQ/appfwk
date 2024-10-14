/**
 * @file dummy_module_example.cxx
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/DAQModule.hpp"

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

  // Init
  TLOG() << "Calling init on modules...";
  dummy_module->init(nullptr);

  TLOG() << "Calling stuff on module...";
  dummy_module->execute_command("stuff", "RUNNING");

  TLOG() << "Calling bad_stuff on module...";
  try {
    dummy_module->execute_command("bad_stuff", "RUNNING");
    TLOG() << "Should have thrown exception";
  } catch (GeneralDAQModuleIssue&) {
    TLOG() << "Exception thrown as expected";
  }

  TLOG() << "Test complete";
}
