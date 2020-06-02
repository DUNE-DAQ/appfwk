/**
 * @file QueryResponseCommandFacility.hh
 *
 * QueryResponseCommandFacility is a simple CommandFacility plugin which is
 * designed to take commands from standard input for testing purposes.
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_SRC_QUERYRESPONSECOMMANDFACILITY_HH_
#define APP_FRAMEWORK_SRC_QUERYRESPONSECOMMANDFACILITY_HH_

#include "app-framework/DAQProcess.hh"

#include "app-framework/CommandFacility.hh"

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
   * @return Status code. 0 if terminated by quit command, -1 if an exception occurs
   */
  int listen(DAQProcess* theProcess) override;
  virtual ~QueryResponseCommandFacility();
};
} // namespace appframework

#endif // APP_FRAMEWORK_SRC_QUERYRESPONSECOMMANDFACILITY_HH_
