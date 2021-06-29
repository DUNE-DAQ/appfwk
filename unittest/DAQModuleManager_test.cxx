/**
 * @file DAQModuleManager_test.cxx DAQModuleManager class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "cmdlib/cmd/Nljs.hpp"

#include "appfwk/DAQModuleManager.hpp"
#include "appfwk/app/Nljs.hpp"

#define BOOST_TEST_MODULE DAQModuleManager_test // NOLINT

#include "boost/test/unit_test.hpp"

#include <string>
#include <type_traits>

BOOST_AUTO_TEST_SUITE(DAQModuleManager_test)

using namespace dunedaq::appfwk;

BOOST_AUTO_TEST_CASE(Construct)
{
  auto mgr = DAQModuleManager();
}

BOOST_AUTO_TEST_CASE(Initialized)
{
  auto mgr = DAQModuleManager();
  BOOST_REQUIRE_EQUAL(mgr.initialized(), false);

  dunedaq::appfwk::app::Init init;
  nlohmann::json init_data;
  to_json(init_data, init);
  dunedaq::cmdlib::cmd::Command cmd;
  cmd.id = "init";
  cmd.data = init_data;
  nlohmann::json cmd_data;
  to_json(cmd_data, cmd);
  mgr.execute(cmd_data);

  BOOST_REQUIRE_EQUAL(mgr.initialized(), true);
}

BOOST_AUTO_TEST_CASE(Stats)
{
  auto mgr = DAQModuleManager();
  dunedaq::opmonlib::InfoCollector ic;
  mgr.gather_stats(ic, 0);
}

BOOST_AUTO_TEST_SUITE_END()
