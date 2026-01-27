/**
 * @file DAQModule_test.cxx DAQModule class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/ConfigurationManager.hpp"
#include "appfwk/DAQModule.hpp"

#define BOOST_TEST_MODULE DAQModule_test // NOLINT

#include "boost/test/unit_test.hpp"
#include "nlohmann/json.hpp"

#include <set>
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

  void init(std::shared_ptr<ConfigurationManager>) final {}

  void do_stuff(const CommandData_t& /*data*/) {}
  void do_other_stuff(const CommandData_t& /*data*/) {}
};

class RegisterCommandDAQModule : public DAQModule
{
public:
  explicit RegisterCommandDAQModule(std::string const& name)
    : DAQModule(name)
  {
    register_command("stuff", &RegisterCommandDAQModule::do_stuff);
  }

  void init(std::shared_ptr<ConfigurationManager>) final {}

  void try_register(std::string cmd) { register_command(cmd, &RegisterCommandDAQModule::do_stuff); }

  void do_stuff(const CommandData_t& /*data*/) {}
};

class GoodDAQModule : public DAQModule
{
public:
  explicit GoodDAQModule(std::string const& name)
    : DAQModule(name)
  {
    register_command("stuff", &GoodDAQModule::do_stuff);
  }

  void init(std::shared_ptr<ConfigurationManager>) final {}

  void do_stuff(const CommandData_t& /*data*/) {}
};

class AnyDAQModule : public DAQModule
{
public:
  explicit AnyDAQModule(std::string const& name)
    : DAQModule(name)
  {
    register_command("no_stuff", &AnyDAQModule::do_stuff);
    register_command("any_stuff", &AnyDAQModule::do_stuff);
    register_command("any_stuff_oops", &AnyDAQModule::do_stuff);
  }

  void init(std::shared_ptr<ConfigurationManager>) final {}

  void do_stuff(const CommandData_t& /*data*/) {}
};
} // namespace daqmoduletest

BOOST_AUTO_TEST_CASE(Construct)
{
  daqmoduletest::GoodDAQModule gdm("construct_test_good");
  BOOST_REQUIRE_THROW(daqmoduletest::BadDAQModule bdm("construct_test_bad"), CommandRegistrationFailedMessage);
}

BOOST_AUTO_TEST_CASE(Commands)
{
  daqmoduletest::GoodDAQModule gdm("command_test");

  BOOST_REQUIRE(gdm.has_command("stuff"));
  auto valid_commands = gdm.get_commands();
  BOOST_REQUIRE_EQUAL(valid_commands.size(), 1);
  BOOST_REQUIRE_EQUAL(valid_commands[0], "stuff");

  gdm.execute_command("stuff", {});
  BOOST_REQUIRE_THROW(gdm.execute_command("other_stuff", {}), UnknownCommand);

  daqmoduletest::AnyDAQModule adm("command_test");
  BOOST_REQUIRE(adm.has_command("any_stuff"));
  BOOST_REQUIRE(adm.has_command("no_stuff"));
  BOOST_REQUIRE(adm.has_command("any_stuff_oops"));
  valid_commands = adm.get_commands();
  BOOST_REQUIRE_EQUAL(valid_commands.size(), 3);

  adm.execute_command("any_stuff", {});
  adm.execute_command("any_stuff", {});
  adm.execute_command("no_stuff", {});
  adm.execute_command("no_stuff", {});
  adm.execute_command("any_stuff_oops", {});
  adm.execute_command("any_stuff_oops", {});
}

BOOST_AUTO_TEST_CASE(MakeModule)
{
  BOOST_REQUIRE_EXCEPTION(make_module("not_a_real_plugin_name", "error_test"),
                          DAQModuleCreationFailed,
                          [&](DAQModuleCreationFailed) { return true; });
}

BOOST_AUTO_TEST_CASE(RegisterCommand)
{
  daqmoduletest::RegisterCommandDAQModule rdm("register_command_test");

  // This is allowed
  rdm.try_register("before_inhibit");
  rdm.set_command_registration_allowed(false);
  BOOST_REQUIRE_EXCEPTION(rdm.try_register("after_inhibit"),
                          CommandRegistrationFailedMessage,
                          [&](CommandRegistrationFailedMessage) { return true; });
  BOOST_REQUIRE(rdm.has_command("stuff"));
  BOOST_REQUIRE(rdm.has_command("before_inhibit"));
  BOOST_REQUIRE(!rdm.has_command("after_inhibit"));
}

BOOST_AUTO_TEST_SUITE_END()
