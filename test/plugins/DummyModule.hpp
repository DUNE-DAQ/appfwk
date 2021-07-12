/**
 * @file DummyModule.hpp
 *
 * DummyModule is a simple DAQModule implementation that responds to a "stuff" command with a log message.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_TEST_PLUGINS_DUMMYMODULE_HPP_
#define APPFWK_TEST_PLUGINS_DUMMYMODULE_HPP_

#include "appfwk/DAQModule.hpp"

#include "ers/ers.hpp"

#include <string>
#include <vector>

namespace dunedaq {

// Disable coverage collection LCOV_EXCL_START
ERS_DECLARE_ISSUE_BASE(appfwk,
                       DummyModuleUpdate,
                       appfwk::GeneralDAQModuleIssue,
                       message,
                       ((std::string)name),
                       ((std::string)message))
// Re-enable coverage collection LCOV_EXCL_STOP

namespace appfwk {

class DummyParentModule : public DAQModule
{
public:
  explicit DummyParentModule(const std::string& name)
    : DAQModule(name)
  {
    register_command("stuff", &DummyParentModule::do_stuff);
  }

  void init(const nlohmann::json&) final {}

  virtual void do_stuff(const data_t& /*data*/) = 0;
};

class DummyModule : public DummyParentModule
{
public:
  explicit DummyModule(const std::string& name)
    : DummyParentModule(name)
  {
    register_command("bad_stuff", &DummyModule::do_bad_stuff);
  }

  void do_bad_stuff(const data_t&) { throw DummyModuleUpdate(ERS_HERE, get_name(), "DummyModule do_bad_stuff"); }

  void do_stuff(const data_t& /*data*/) override
  {
    ers::info(DummyModuleUpdate(ERS_HERE, get_name(), "DummyModule do_stuff"));
  };
};

} // namespace appfwk
} // namespace dunedaq

#endif // APPFWK_TEST_PLUGINS_DUMMYMODULE_HPP_
