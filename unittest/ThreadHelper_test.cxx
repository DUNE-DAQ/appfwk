/**
 *
 * @file ThreadHelper_test.cxx ThreadHelper class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/ThreadHelper.hpp"

#define BOOST_TEST_MODULE ThreadHelper_test // NOLINT
#include <boost/test/unit_test.hpp>

#include <boost/asio/signal_set.hpp>

#include <chrono>
#include <memory>

namespace {

void
DoSomething(std::atomic<bool>&)
{
  int nseconds = 5;
  BOOST_TEST_MESSAGE("This function will just sleep for " << nseconds << " second(s) and then return");
  std::this_thread::sleep_for(std::chrono::seconds(nseconds));
}

} // namespace

BOOST_AUTO_TEST_CASE(sanity_checks)
{

  std::unique_ptr<dunedaq::appfwk::ThreadHelper> umth_ptr = nullptr;

  auto starttime = std::chrono::steady_clock::now();
  BOOST_REQUIRE_NO_THROW(umth_ptr = std::make_unique<dunedaq::appfwk::ThreadHelper>(DoSomething));
  auto construction_time_in_ms =
    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - starttime).count();
  BOOST_TEST_MESSAGE("Construction time was " << construction_time_in_ms << " ms");

  starttime = std::chrono::steady_clock::now();
  BOOST_REQUIRE_NO_THROW(umth_ptr->start_working_thread());
  auto start_time_in_ms =
    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - starttime).count();
  BOOST_TEST_MESSAGE("Time to call ThreadHelper::start_working_thread() was " << start_time_in_ms << " ms");

  starttime = std::chrono::steady_clock::now();
  BOOST_REQUIRE_NO_THROW(umth_ptr->stop_working_thread());
  auto stop_time_in_ms =
    std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - starttime).count();
  BOOST_TEST_MESSAGE("Time to call ThreadHelper::stop_working_thread() was " << stop_time_in_ms << " ms");
}

BOOST_AUTO_TEST_CASE(inappropriate_transitions, *boost::unit_test::depends_on("sanity_checks"))
{

  dunedaq::appfwk::ThreadHelper umth(DoSomething);
  BOOST_REQUIRE_THROW(umth.stop_working_thread(), dunedaq::appfwk::ThreadingIssue);

  umth.start_working_thread();

  BOOST_REQUIRE_THROW(umth.start_working_thread(), dunedaq::appfwk::ThreadingIssue);

  umth.stop_working_thread();
}

// You'll want this to test case to execute last, for reasons that are obvious
// if you look at its checks

BOOST_AUTO_TEST_CASE(abort_checks, *boost::unit_test::depends_on("inappropriate_transitions"))
{

  {
    dunedaq::appfwk::ThreadHelper umth(DoSomething);
  }
  BOOST_TEST(true,
             "ThreadHelper without having start_working_thread() thread "
             "called destructs without aborting the program, as expected");

  // BOOST_TEST_MESSAGE(
  //     "You should *expect* the program to abort in a moment, since we're "
  //     "destructing a ThreadHelper instance after calling "
  //     "start_working_thread() but before calling stop_working_thread_()");

  // {
  //   dunedaq::appfwk::ThreadHelper umth(DoSomething);
  //   umth.start_working_thread();
  // }
}
