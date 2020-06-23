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
class DAQProcess; // forward declaration

/**
 * @brief Interface needed by Application Framework for CCM command handling
 */
class CommandFacility
{
public:
  /**
   * @brief Singleton pattern; get a handle to the CommandFacility
   * @return Reference to the CommandFacility
   */
  static CommandFacility& handle()
  {
    if (!handle_)
      handle_.reset(new CommandFacility());
    return *handle_;
  }

  /**
   * @brief Set the pointer returned by the handle() function
   * @param handle Handle to a loaded CommandFacility plugin
   */
  static void setHandle(std::unique_ptr<CommandFacility>&& handle) { handle_ = std::move(handle); }
  /**
   * @brief Perform basic setup actions needed by the CommandFacility, using
   * command-line arguments and environment variables
   * @param args Command-line arguments to the CommandFacility
   */
  virtual void setup(std::vector<std::string> /*args*/) {}
  /**
   * @brief Listen for commands and relay them to the given DAQProcess
   * @param process DAQProcess to relay commands to
   * @return Return code for application
   *
   * This function should block for the lifetime of the DAQ Application, calling
   * DAQProcess::execute_command as necessary
   */
  virtual int listen(DAQProcess* /*process*/) { return 0; }

protected:
  /**
   * Singleton pattern; protected parameterless constructor
   */
  CommandFacility() {}

private:
  static std::unique_ptr<CommandFacility> handle_; ///< Singleton pattern, handle to CommandFacility
};

/**
 * @brief Instantiate a CommandFacility from a plugin
 * @param facility_name Name of the CommandFacility plugin to load
 * @return Pointer to loaded CommandFacility from plugin
 */
inline std::unique_ptr<CommandFacility>
makeCommandFacility(std::string const& facility_name)
{
  static cet::BasicPluginFactory bpf("duneCommandFacility", "make");

  return bpf.makePlugin<std::unique_ptr<CommandFacility>>(facility_name);
}

} // namespace dunedaq::appfwk

#endif // APPFWK_INCLUDE_APPFWK_COMMANDFACILITY_HPP_
