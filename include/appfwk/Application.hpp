/**
 * @file Application.hpp Loads and distributes commands to DAQModules
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_APPLICATION_HPP_
#define APPFWK_INCLUDE_APPFWK_APPLICATION_HPP_

#include "appfwk/DAQModuleManager.hpp"
#include "appfwk/rest/Api.hpp"
#include "appfwk/NamedObject.hpp"
#include "appfwk/StateObject.hpp"
#include "appfwk/cmd/Structs.hpp"

#include "cmdlib/CommandedObject.hpp"
#include "cmdlib/CommandFacility.hpp"

#include "opmonlib/InfoProvider.hpp"
#include "opmonlib/InfoManager.hpp"

#include "ers/Issue.hpp"
#include "nlohmann/json.hpp"

#include <string>
#include <atomic>
#include <stdint.h>

namespace dunedaq {

/**
 * @brief A generic Application ERS Issue
 */
ERS_DECLARE_ISSUE(appfwk,                                                   ///< Namespace
                  ApplicationNotInitialized,                           ///< Issue class name
                  "Application " << name << " has not been initialized yet.", ///< Message
                  ((std::string)name)                                      ///< Message parameters
)

/**
 * @brief An invalid command was received
 */
ERS_DECLARE_ISSUE(appfwk,                                                   ///< Namespace
                  InvalidCommand,                           ///< Issue class name
                  "Command " << cmdid << " not allowed. state: " << state << ", error: " << err << ", busy: " << busy, ///< Message
                  ((std::string)cmdid)                                      ///< Message parameters
                  ((std::string)state)                                      ///< Message parameters
                  ((bool) err)                                                ///< Message parameters
                  ((bool) busy)                                                ///< Message parameters
)

namespace appfwk {

class Application : public cmdlib::CommandedObject, public opmonlib::InfoProvider, public StateObject, public NamedObject
{
public:
  using dataobj_t = nlohmann::json;

  Application(std::string app_name, std::string partition_name, std::uint16_t rest_port, std::string cmdlibimpl, std::string opmonlibimpl);

  // Initialize the application services
  void init();

  // Start the main run loop
  void run(std::atomic<bool>& end_marker);

  // Execute a properly structured command
  void execute(const dataobj_t& cmd_data);

  // Gether the opmon information

  void gather_stats(opmonlib::InfoCollector& ic, int level);

  // Check whether the command can be accepted
  bool is_cmd_valid(const dataobj_t& cmd_data);

private:
  std::string m_partition;
  opmonlib::InfoManager m_info_mgr;
  rest::Api m_rest_api;
  std::atomic<bool> m_busy;
  std::atomic<bool> m_error;
  bool m_initialized;
  DAQModuleManager m_mod_mgr;
  std::shared_ptr<cmdlib::CommandFacility> m_cmd_fac;

};

} // namespace appfwk
} // namespace dunedaq
#endif // APPFWK_INCLUDE_APPFWK_APPLICATION_HPP_
