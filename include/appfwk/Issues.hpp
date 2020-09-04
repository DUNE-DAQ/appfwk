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

#ifndef DUNEDAQ_APPFWK_ISSUES_HPP
#define DUNEDAQ_APPFWK_ISSUES_HPP

#include "ers/ers.h"
#include <string>

namespace dunedaq {

    // Throw if a CLI receives unexpected usage.  The "what" arg
    // should explain the nature of the error.  Use ERS_INFO() or
    // similar to provide lengthy usage.
    ERS_DECLARE_ISSUE(appfwk, BadCliUsage, what, ((std::string)what))

    // Throw if a URI is given which is not supported by the code.
    // Provide the erroneous URI as arg.
    ERS_DECLARE_ISSUE(appfwk, UnsupportedUri,
                      "Unsupported URI: " << uri,
                      ((std::string)uri))

    // Throw if a file can not be opened.  Provide "mode" of "reading"
    // or "writing" and provide erroneous filename as args.
    ERS_DECLARE_ISSUE(appfwk, BadFile,
                      "Can not open file for " << mode << ": " << filename,
                      ((std::string)filename)
                      ((std::string)mode))

    // Throw if a stream of data is exhausted.  Use in methods that
    // are given semantics that imply the stream is not expected to
    // become exhausted.  Ie, GetNext() may throw but something like
    // GetNext(return_if_exhausted) -> value must not.  Name the
    // stream and describe the error in the args.
    ERS_DECLARE_ISSUE(appfwk, StreamExhausted,
                      "Stream exhausted: " << stream_name << " err: " << err,
                      ((std::string)stream_name)
                      ((std::string)err))

    // Throw if a stream of data being consumed has values which are
    // not expected.  Give the stream name and error message as args.
    ERS_DECLARE_ISSUE(appfwk, StreamCorrupt,
                      "Stream corrupt: " << stream_name << " err: " << err,
                      ((std::string)stream_name)
                      ((std::string)err))                  

    // Throw if an operation driven by a schema fails due to data not
    // following the schema.  This can be considered a special case of
    // a corrupt stream.  Describe the error in the "what" arg.
    ERS_DECLARE_ISSUE(appfwk, SchemaError,
                      "Schema error: " << what,
                      ((std::string)what))

    // Throw if an operation is the result of logically inconsistent
    // code.  Describe the error in the "what" arg.
    ERS_DECLARE_ISSUE(appfwk, InternalError,
                      "Internal error: " << what,
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

}


#endif
