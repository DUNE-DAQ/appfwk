
#ifndef APP_FRAMEWORK_TEST_APP_FRAMEWORK_DUMMYMODULE_HPP_
#define APP_FRAMEWORK_TEST_APP_FRAMEWORK_DUMMYMODULE_HPP_


#include "appfwk/DAQModule.hpp"
namespace dunedaq::appfwk {

class DummyParentModule : public DAQModule
{
public:
  explicit DummyParentModule(const std::string& name) : DAQModule(name) {
    register_command("stuff", &DummyParentModule::do_stuff);
  }


  virtual void do_stuff(const std::vector<std::string>& args) {
    std::cout << "Parent stuff" << std::endl;
  };

};


class DummyModule : public DummyParentModule
{
public:
  explicit DummyModule(const std::string& name) : DummyParentModule(name) {
  }


  virtual void do_stuff(const std::vector<std::string>& args) override {
    std::cout << "Dummy stuff" << std::endl;
  };

};

}

#endif // APP_FRAMEWORK_TEST_APP_FRAMEWORK_DUMMYMODULE_HPP_
