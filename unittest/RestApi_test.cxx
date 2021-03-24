/**
 * @file Api_test.cxx Api class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/rest/Api.hpp"

#define BOOST_TEST_MODULE RestApi_test // NOLINT

#include "boost/test/unit_test.hpp"

#include <string>
#include <type_traits>

BOOST_AUTO_TEST_SUITE(RestApi_test)

BOOST_AUTO_TEST_CASE(RestApi)
{

  BOOST_REQUIRE(!std::is_copy_constructible_v<dunedaq::rest::Api>);
  BOOST_REQUIRE(!std::is_copy_assignable_v<dunedaq::rest::Api>);
  BOOST_REQUIRE(std::is_move_constructible_v<dunedaq::rest::Api>);
  BOOST_REQUIRE(std::is_move_assignable_v<dunedaq::rest::Api>);

  // RestApi fails on permission errors
  auto invalidport = dunedaq::rest::Api(1);
  try {
    invalidport.start();
    BOOST_TEST_REQUIRE(false, "Test failure: starting API on port 1 did not throw exception");
  } catch (const dunedaq::rest::RestApiStartFailed& ex) {
    BOOST_TEST_REQUIRE(true, "API fails to open port 1");
  }
  // RestApis can be stopped even when they didn't start properly
  try {
    invalidport.stop();
    BOOST_TEST_REQUIRE(!invalidport.is_started());
    BOOST_TEST_REQUIRE(!invalidport.start_failed());
  } catch (...) { // NOLINT
    BOOST_TEST_REQUIRE(false, "Test failure: unexpected exception thrown while stopping API");
  }

  // RestApi can open on port 0 (random port)
  auto api = dunedaq::rest::Api(0);
  try {
    api.start();
    BOOST_TEST_REQUIRE(api.is_started());
    BOOST_TEST_REQUIRE(!api.start_failed());
    BOOST_TEST_REQUIRE(api.get_port() != 0);
  } catch (const dunedaq::rest::RestApiStartFailed& ex) {
    BOOST_TEST_REQUIRE(false, "Test failure: unexpected API Start exception thrown");
  } catch (...) { // NOLINT
    BOOST_TEST_REQUIRE(false, "Test failure: unexpected exception thrown while starting API");
  }

  // RestApi cannot be started twice
  try {
    api.start();
    BOOST_TEST_REQUIRE(false, "Test failure: starting API twice did not throw exception");
  } catch (const dunedaq::rest::RestApiAlreadyRunning& ex) {
    BOOST_TEST_REQUIRE(true, "API refuses to open twice");
  } catch (...) { // NOLINT
    BOOST_TEST_REQUIRE(false, "Test failure: unexpected exception thrown while starting API twice");
  }

  // RestApis can be stopped
  try {
    api.stop();
    BOOST_TEST_REQUIRE(!api.is_started());
    BOOST_TEST_REQUIRE(!api.start_failed());
  } catch (...) { // NOLINT
    BOOST_TEST_REQUIRE(false, "Test failure: unexpected exception thrown while stopping API");
  }

  std::cout << "am here" << std::endl;

  // a started API auto-stops
  try {
    auto unloved = new dunedaq::rest::Api(0);
    unloved->start();
    delete unloved;
  } catch (...) { // NOLINT
    BOOST_TEST_REQUIRE(false, "Test failure: API cannot handle implicit close");
  }
}

BOOST_AUTO_TEST_SUITE_END()
