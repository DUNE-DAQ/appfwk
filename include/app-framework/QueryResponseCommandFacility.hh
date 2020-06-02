/**
 * @file QueryResponseCommandFacility class interface
 *
 * QueryResponseCommandFacility is a simple CommandFacility plugin which is
 * designed to take commands from standard input for testing purposes.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUERYRESPONSECOMMANDFACILITY_HH_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUERYRESPONSECOMMANDFACILITY_HH_

#include "app-framework/DAQProcess.hh"

#include "app-framework/Services/CommandFacility.hh"

namespace appframework {
/**
 * @brief QueryResponseCommandFacility is a CommandFacility plugin for running a
 * DAQ Application in an interactive session
 */
class QueryResponseCommandFacility : public CommandFacility
{
public:
  /**
   * @brief Listen for commands from stdin, and pass them to the attached
   * DAQProcess
   * @param theProcess Pointer to the DAQProcess instance which will distribute
   * received commands
   */
  int listen(DAQProcess* theProcess) override;
  virtual ~QueryResponseCommandFacility();
};
} // namespace appframework

#endif // APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_QUERYRESPONSECOMMANDFACILITY_HH_
