#include "appfwk/DAQModule.hpp"



namespace dunedaq::appfwk {

class DummyModule : public DAQModule
{
public:
  explicit DummyModule(const std::string& name) : DAQModule(name) {
    register_command("stuff", &DummyModule::do_stuff);
  }


  void do_stuff(const std::vector<std::string>& args) {};

};

}