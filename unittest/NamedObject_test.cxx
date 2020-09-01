/**
 * @file Named_test.cxx Named/NamedObject class Unit Tests
 *
 * The tests here primarily confirm that the move and copy semantics
 * are what we expect them to be
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/NamedObject.hpp"

#define BOOST_TEST_MODULE NamedObject_test // NOLINT

#include <boost/test/unit_test.hpp>
#include <string>
#include <type_traits>

BOOST_AUTO_TEST_SUITE(NamedObject_test)

BOOST_AUTO_TEST_CASE(Named)
{

  class DerivesFromNamed : public dunedaq::appfwk::Named {
    public:
      explicit DerivesFromNamed(const std::string& name) : myname_(name) {}
      virtual const std::string& get_name() const { return myname_; };
    private:
      std::string myname_;
  };

  BOOST_REQUIRE( !std::is_copy_constructible_v<DerivesFromNamed> );
  BOOST_REQUIRE( !std::is_copy_assignable_v<DerivesFromNamed> );
  BOOST_REQUIRE( std::is_move_constructible_v<DerivesFromNamed> );
  BOOST_REQUIRE( std::is_move_assignable_v<DerivesFromNamed> );
}

BOOST_AUTO_TEST_CASE(NamedObject)
{
  class DerivesFromNamedObject : public dunedaq::appfwk::NamedObject {
    // No implementation needed
  };

  BOOST_REQUIRE( !std::is_copy_constructible_v<DerivesFromNamedObject> );
  BOOST_REQUIRE( !std::is_copy_assignable_v<DerivesFromNamedObject> );
  BOOST_REQUIRE( std::is_move_constructible_v<DerivesFromNamedObject> );
  BOOST_REQUIRE( std::is_move_assignable_v<DerivesFromNamedObject> );
}

BOOST_AUTO_TEST_SUITE_END()
