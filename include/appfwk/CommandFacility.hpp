/**
 * @file CommandFacility.hpp CommandFacility interface
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_COMMANDFACILITY_HPP_
#define APPFWK_INCLUDE_APPFWK_COMMANDFACILITY_HPP_

#include <cetlib/BasicPluginFactory.h>
#include <cetlib/compiler_macros.h>

#include <iostream>
#include <list>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#ifndef EXTERN_C_FUNC_DECLARE_START
#define EXTERN_C_FUNC_DECLARE_START                                                                                    \
  extern "C"                                                                                                           \
  {
#endif

/**
 * @brief Declare the function that will be called by the plugin loader
 * @param klass Class to be defined as a DUNE Command Facility
 */
#define DEFINE_DUNE_COMMAND_FACILITY(klass)                                                                            \
  EXTERN_C_FUNC_DECLARE_START                                                                                          \
  std::unique_ptr<dunedaq::appfwk::CommandFacility> make()                                                             \
  {                                                                                                                    \
    return std::unique_ptr<dunedaq::appfwk::CommandFacility>(new klass());                                             \
  }                                                                                                                    \
  }

namespace dunedaq::appfwk {
class DAQModuleManager; // forward declaration

/**
 * @brief Interface needed by Application Framework for command handling
 */
class CommandFacility
{
public:
  CommandFacility(std::string /*uri*/) {}

  virtual void run(DAQModuleManager& /*process*/) const = 0;

private:


};




std::shared_ptr<CommandFacility>
makeCommandFacility(std::string const& uri)
{
    auto sep = uri.find("://");
    std::string scheme;
    if (sep == std::string::npos) { // simple path
        scheme = "file";
    }
    else {                  // with scheme
        scheme = uri.substr(0, sep);
    }
    std::string plugin_name = scheme + "CommandFacility";
    static cet::BasicPluginFactory bpf("duneCommandFacility", "make");
    return bpf.makePlugin<std::shared_ptr<CommandFacility>>(plugin_name, uri);
}

}

#endif // APPFWK_INCLUDE_APPFWK_COMMANDFACILITY_HPP_