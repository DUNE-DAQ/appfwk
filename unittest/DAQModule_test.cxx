/**
 * @file DAQModule_test.cxx DAQModule class Unit Tests
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "appfwk/DAQModule.hpp"
#include "appfwk/DAQModuleHelper.hpp"
#include "appfwk/Issues.hpp"
#include "appfwk/QueueRegistry.hpp"
#include "appfwk/app/Nljs.hpp"

#define BOOST_TEST_MODULE DAQModule_test // NOLINT

#include "boost/test/unit_test.hpp"
#include "nlohmann/json.hpp"

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

  void init(const nlohmann::json&) final {}

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

  void init(const nlohmann::json&) final {}

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

  BOOST_REQUIRE(gdm.has_command("stuff"));
  auto valid_commands = gdm.get_commands();
  BOOST_REQUIRE_EQUAL(valid_commands.size(), 1);
  BOOST_REQUIRE_EQUAL(valid_commands[0], "stuff");

  dunedaq::opmonlib::InfoCollector ic;
  gdm.get_info(ic, 0);

  gdm.execute_command("stuff", {});
  BOOST_REQUIRE_THROW(gdm.execute_command("other_stuff", {}), UnknownCommand);
}

BOOST_AUTO_TEST_CASE(MakeModule)
{
  BOOST_REQUIRE_EXCEPTION(make_module("not_a_real_plugin_name", "error_test"),
                          DAQModuleCreationFailed,
                          [&](DAQModuleCreationFailed) { return true; });
}

BOOST_AUTO_TEST_CASE(QueueInfo)
{

  app::ModInit data;
  app::QueueInfo queue_info{ "test_queue", "output", "out" };
  data.qinfos.push_back(queue_info);
  nlohmann::json json;
  to_json(json, data);

  auto infos = queue_infos(json);
  BOOST_REQUIRE_EQUAL(infos.size(), 1);

  auto index = queue_index(json, std::vector<std::string>{ "output" });
  BOOST_REQUIRE_EQUAL(index.size(), 1);
  BOOST_REQUIRE_EQUAL(index["output"].inst, queue_info.inst);
  BOOST_REQUIRE_EQUAL(index["output"].name, queue_info.name);
  BOOST_REQUIRE_EQUAL(index["output"].dir, queue_info.dir);

  BOOST_REQUIRE_EXCEPTION(queue_index(json, std::vector<std::string>{ "output", "ERROR" }),
                          InvalidSchema,
                          [&](InvalidSchema) { return true; });
}

BOOST_AUTO_TEST_SUITE_END()
