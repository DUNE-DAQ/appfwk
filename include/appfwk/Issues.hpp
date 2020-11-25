/** @file Issues.hpp Provide set of common ERS issues.
 *
 * Use like
 *
 *     throw ExceptionType(ERS_HERE, ...args.if.any...);
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APPFWK_INCLUDE_APPFWK_ISSUES_HPP_
#define APPFWK_INCLUDE_APPFWK_ISSUES_HPP_

#include "ers/ers.h"
#include <string>

namespace dunedaq {

    // Throw if a CLI receives unexpected usage.  The "what" arg
    // should explain the nature of the error.  Use ERS_INFO() or
    // similar to provide lengthy usage.
    ERS_DECLARE_ISSUE(appfwk, BadCliUsage, what, ((std::string)what))

    // Throw if an operation driven by a schema fails due to data not
    // following the schema.  This can be considered a special case of
    // a corrupt stream.  Describe the error in the "what" arg.
    ERS_DECLARE_ISSUE(appfwk, SchemaError,
                      "Schema error: " << what,
                      ((std::string)what))

    // Throw if a lookup of a component fails.
    ERS_DECLARE_ISSUE(appfwk, MissingComponent,
                      "No such component: " << what,
                      ((std::string)what))

    // Throw if a command is received for which there is no handler
    // implemented.
    ERS_DECLARE_ISSUE(appfwk, UnexpectedCommand,
                      "Unexpected command: " << cmd << " received by " << who,
                      ((std::string)cmd)
                      ((std::string)who))

    ERS_DECLARE_ISSUE(appfwk, CommandContractViolation,
                      "Contract " << who
                      << " violation with command: "
                      << cmd << ": " << what,
                      ((std::string)what)
                      ((std::string)cmd)
                      ((std::string)who))

} // namespace dunedaq


#endif  // APPFWK_INCLUDE_APPFWK_ISSUES_HPP_
