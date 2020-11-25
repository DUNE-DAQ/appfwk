/**
 * @file DAQModule_test.cxx DAQModule class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/DAQModule.hpp"
#include "appfwk/QueueRegistry.hpp"

#define BOOST_TEST_MODULE DAQModule_test // NOLINT

#include <boost/test/unit_test.hpp>
#include <nlohmann/json.hpp>

#include <string>
#include <vector>

constexpr auto queue_timeout = std::chrono::milliseconds(10);
using namespace dunedaq::appfwk;

BOOST_AUTO_TEST_SUITE(DAQModule_test)

namespace daqmoduletest {
class BadDAQModule : public DAQModule
{
public:
  explicit BadDAQModule(std::string const& name)
    : DAQModule(name)
  {
    register_command("stuff", &BadDAQModule::do_stuff);

    // THIS WILL FAIL
    register_command("stuff", &BadDAQModule::do_other_stuff);
  }

  void init(const nlohmann::json& ) final {}

  void do_stuff(const data_t& /*data*/) {}
  void do_other_stuff(const data_t& /*data*/) {}
};

class GoodDAQModule : public DAQModule
{
public:
  explicit GoodDAQModule(std::string const& name)
    : DAQModule(name)
  {
    register_command("stuff", &GoodDAQModule::do_stuff);
  }

  void init(const nlohmann::json& ) final {}

  void do_stuff(const data_t& /*data*/) {}
};
} // namespace daqmoduletest

BOOST_AUTO_TEST_CASE(Construct)
{
  daqmoduletest::GoodDAQModule gdm("construct_test_good");
  BOOST_REQUIRE_THROW(daqmoduletest::BadDAQModule bdm("construct_test_bad"), CommandRegistrationFailed);
}

BOOST_AUTO_TEST_CASE(Commands)
{

  daqmoduletest::GoodDAQModule gdm("command_test");

  gdm.execute_command("stuff", {});
  BOOST_REQUIRE_THROW(gdm.execute_command("other_stuff", {}), UnknownCommand);
}

BOOST_AUTO_TEST_SUITE_END()
