/**
 * @file StateObject_test.cxx StateObject class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/StateObject.hpp"

#define BOOST_TEST_MODULE StateObject_test // NOLINT

#include "boost/test/unit_test.hpp"

#include <string>
#include <type_traits>

BOOST_AUTO_TEST_SUITE(StateObject_test)

BOOST_AUTO_TEST_CASE(StateObject)
{
  class DerivesFromStateObject : public dunedaq::appfwk::StateObject
  {
    // No implementation needed
  };

  BOOST_REQUIRE(!std::is_copy_constructible_v<DerivesFromStateObject>);
  BOOST_REQUIRE(!std::is_copy_assignable_v<DerivesFromStateObject>);
  BOOST_REQUIRE(!std::is_move_constructible_v<DerivesFromStateObject>);
  BOOST_REQUIRE(!std::is_move_assignable_v<DerivesFromStateObject>);

  // StateObject correctly implements the initial state
  auto obj = dunedaq::appfwk::StateObject("none");
  BOOST_REQUIRE(obj.get_state() == "none");

  // StateObject correctly sets a new state
  obj.set_state("newstate");
  BOOST_REQUIRE(obj.get_state() == "newstate");

  // set_state works more than one time
  obj.set_state("newstate2");
  BOOST_REQUIRE(obj.get_state() == "newstate2");

  // StateObject can have an empty string for state
  auto obj2 = dunedaq::appfwk::StateObject("");
  BOOST_REQUIRE(obj2.get_state() == "");
}

BOOST_AUTO_TEST_SUITE_END()
