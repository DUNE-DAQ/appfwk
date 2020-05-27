/**
 *
 * @file DAQModuleThreadHelper class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "app-framework-base/DAQModules/DAQModuleThreadHelper.hh"

#define BOOST_TEST_MODULE DAQModuleThreadHelper_test // NOLINT
#include <boost/test/unit_test.hpp>

#include <boost/asio/signal_set.hpp>

#include <chrono>
#include <csignal>
#include <thread>

namespace {

void
DoSomething()
{
  int nseconds = 5;
  BOOST_TEST_MESSAGE("This function will just sleep for "
                     << nseconds << " second(s) and then return");
  std::this_thread::sleep_for(std::chrono::seconds(nseconds));
}

} // namespace

BOOST_AUTO_TEST_CASE(sanity_checks)
{

  std::unique_ptr<appframework::DAQModuleThreadHelper> umth_ptr = nullptr;

  auto starttime = std::chrono::steady_clock::now();
  BOOST_REQUIRE_NO_THROW(
    umth_ptr =
      std::make_unique<appframework::DAQModuleThreadHelper>(DoSomething));
  auto construction_time_in_ms =
    std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - starttime)
      .count();
  BOOST_TEST_MESSAGE("Construction time was " << construction_time_in_ms
                                              << " ms");

  starttime = std::chrono::steady_clock::now();
  BOOST_REQUIRE_NO_THROW(umth_ptr->start_working_thread_());
  auto start_time_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::steady_clock::now() - starttime)
                            .count();
  BOOST_TEST_MESSAGE(
    "Time to call DAQModuleThreadHelper::start_working_thread_() was "
    << start_time_in_ms << " ms");

  starttime = std::chrono::steady_clock::now();
  BOOST_REQUIRE_NO_THROW(umth_ptr->stop_working_thread_());
  auto stop_time_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                           std::chrono::steady_clock::now() - starttime)
                           .count();
  BOOST_TEST_MESSAGE(
    "Time to call DAQModuleThreadHelper::stop_working_thread_() was "
    << stop_time_in_ms << " ms");
}

BOOST_AUTO_TEST_CASE(inappropriate_transitions,
                     *boost::unit_test::depends_on("sanity_checks"))
{

  appframework::DAQModuleThreadHelper umth(DoSomething);
  BOOST_REQUIRE_THROW(umth.stop_working_thread_(), std::runtime_error);

  umth.start_working_thread_();

  BOOST_REQUIRE_THROW(umth.start_working_thread_(), std::runtime_error);

  umth.stop_working_thread_();
}

// You'll want this to test case to execute last, for reasons that are obvious
// if you look at its checks

BOOST_AUTO_TEST_CASE(abort_checks,
                     *boost::unit_test::depends_on("inappropriate_transitions"))
{

  {
    appframework::DAQModuleThreadHelper umth(DoSomething);
  }
  BOOST_TEST(
    true,
    "DAQModuleThreadHelper without having start_working_thread_() thread "
    "called destructs without aborting the program, as expected");

  // BOOST_TEST_MESSAGE(
  //     "You should *expect* the program to abort in a moment, since we're "
  //     "destructing a DAQModuleThreadHelper instance after calling "
  //     "start_working_thread_() but before calling stop_working_thread_()");

  // {
  //   appframework::DAQModuleThreadHelper umth(DoSomething);
  //   umth.start_working_thread_();
  // }
}
