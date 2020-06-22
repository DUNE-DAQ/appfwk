
#ifndef APP_FRAMEWORK_TEST_APP_FRAMEWORK_DUMMYMODULE_HPP_
#define APP_FRAMEWORK_TEST_APP_FRAMEWORK_DUMMYMODULE_HPP_

#include "appfwk/DAQModule.hpp"

#include <ers/ers.h>

namespace dunedaq {

ERS_DECLARE_ISSUE_BASE(appfwk,
                       DummyModuleUpdate,
                       appfwk::GeneralDAQModuleIssue,
                       name << ": " << message,
                       ERS_EMPTY,
                       ((std::string)name)((std::string)message))


namespace appfwk {

class DummyParentModule : public DAQModule
{
public:
  explicit DummyParentModule(const std::string& name) : DAQModule(name) {
    register_command("stuff", &DummyParentModule::do_stuff);
  }

  void init() final {}

  virtual void do_stuff(const std::vector<std::string>& /*args*/)
  {
    ers::info(DummyModuleUpdate(ERS_HERE, get_name(), "DummyParentModule do_stuff"));
  };

};


class DummyModule : public DummyParentModule
{
public:
  explicit DummyModule(const std::string& name) : DummyParentModule(name) {
  }


  virtual void do_stuff(const std::vector<std::string>& /*args*/) override
  {
    ers::info(DummyModuleUpdate(ERS_HERE, get_name(), "DummyModule do_stuff"));
  };

};

} // namespace appfwk
} // namespace dunedaq

#endif // APP_FRAMEWORK_TEST_APP_FRAMEWORK_DUMMYMODULE_HPP_
