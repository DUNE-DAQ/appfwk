/**
 * @file ConfigurationManager_test.cxx ConfigurationManager_test class
 * Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/ConfigurationManager.hpp"
#include "confmodel/Variable.hpp"

#define BOOST_TEST_MODULE ConfigurationManager_test // NOLINT

#include "boost/test/unit_test.hpp"

using namespace dunedaq::appfwk;

BOOST_AUTO_TEST_SUITE(ConfigurationManager_test)

const std::string config_spec{"oksconflibs:test/config/appSession.data.xml"};
const std::string appname{"TestApp"};
const std::string session{"test-session"};

BOOST_AUTO_TEST_CASE(Include)
{
  auto cm = std::make_shared<ConfigurationManager>(config_spec, appname, session);
  cm->load_deferred_db("test/config/dummyInclude.data.xml");

  auto variable = cm->get_dal<dunedaq::confmodel::Variable>("test-variable");
  BOOST_REQUIRE(variable != nullptr);
  BOOST_REQUIRE_EQUAL(variable->get_name(), "test");
}
BOOST_AUTO_TEST_CASE(IncludeTwice)
{
  auto cm = std::make_shared<ConfigurationManager>(config_spec, appname, session);
  cm->load_deferred_db("test/config/dummyInclude.data.xml");
  cm->load_deferred_db("test/config/dummyInclude.data.xml");

  auto variable = cm->get_dal<dunedaq::confmodel::Variable>("test-variable");
  BOOST_REQUIRE(variable != nullptr);
  BOOST_REQUIRE_EQUAL(variable->get_name(), "test");
}


BOOST_AUTO_TEST_CASE(IncludeDuplicate)
{
  auto cm = std::make_shared<ConfigurationManager>(config_spec, appname, session);
  cm->load_deferred_db("test/config/dummyInclude.data.xml");
  BOOST_REQUIRE_THROW(cm->load_deferred_db("test/config/duplicateDummyInclude.data.xml"), FailedInclude);
}
}
