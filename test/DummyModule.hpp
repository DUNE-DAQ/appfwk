/**
 * @file DummyModule.hpp
 *
 * DummyModule is a simple DAQModule implementation that responds to a "stuff" command with a log message.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_TEST_DUMMYMODULE_HPP_
#define APPFWK_TEST_DUMMYMODULE_HPP_

#include "appfwk/DAQModule.hpp"

#include <ers/ers.h>

#include <string>
#include <vector>

namespace dunedaq {

ERS_DECLARE_ISSUE_BASE(appfwk,
                       DummyModuleUpdate,
                       appfwk::GeneralDAQModuleIssue,
                       message,
                       ((std::string)name),
                       ((std::string)message))

namespace appfwk {

class DummyParentModule : public DAQModule
{
public:
  explicit DummyParentModule(const std::string& name)
    : DAQModule(name)
  {
    register_command("stuff", &DummyParentModule::do_stuff);
  }

  void init(const nlohmann::json& ) final {}

  virtual void do_stuff(const data_t& /*data*/)
  {
    ers::info(DummyModuleUpdate(ERS_HERE, get_name(), "DummyParentModule do_stuff"));
  };
};

class DummyModule : public DummyParentModule
{
public:
  explicit DummyModule(const std::string& name)
    : DummyParentModule(name)
  {}

  void do_stuff(const data_t& /*data*/) override
  {
    ers::info(DummyModuleUpdate(ERS_HERE, get_name(), "DummyModule do_stuff"));
  };
};

} // namespace appfwk
} // namespace dunedaq

#endif // APPFWK_TEST_DUMMYMODULE_HPP_
