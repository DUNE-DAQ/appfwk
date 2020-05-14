/**
 *
 * @file UserModuleThreadHelper class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "app-framework-base/UserModules/UserModuleThreadHelper.hh"

#define BOOST_TEST_MODULE UserModuleThreadHelper_test
//#include <boost/test/included/unit_test.hpp>
#include <boost/test/unit_test.hpp>

#include <boost/asio/signal_set.hpp>

#include <chrono>
#include <csignal>
#include <thread>

namespace {

  void DoSomething() {
    int nseconds = 1;
    BOOST_TEST_MESSAGE("This function will just sleep for " << nseconds << " second(s) and then return");
    std::this_thread::sleep_for(std::chrono::seconds(nseconds));
  }

} // namespace ""

//BOOST_AUTO_TEST_SUITE(UserModuleThreadHelper_test)

BOOST_AUTO_TEST_CASE(sanity_checks) {

  std::unique_ptr<appframework::UserModuleThreadHelper> umth_ptr = nullptr;

  BOOST_REQUIRE_NO_THROW(umth_ptr = std::make_unique<appframework::UserModuleThreadHelper>(DoSomething));
  
  BOOST_REQUIRE_NO_THROW(umth_ptr->start_working_thread_());

  BOOST_REQUIRE_NO_THROW(umth_ptr->stop_working_thread_());
}

