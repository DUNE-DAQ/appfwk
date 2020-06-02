/**
 * @file DAQProcess Issues definition
 *
 * This is the files that implements the some generic ERS issues that can be 
 * reported by the DAQPRocess
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQPROCESSISSUES_HH_
#define APP_FRAMEWORK_INCLUDE_APP_FRAMEWORK_DAQPROCESSISSUES_HH_ 

#include <string>

#include <ers/Issue.h>

namespace appframework {

ERS_DECLARE_ISSUE( DAQIssues, 
		   DAQProcessIssue, 
		   "General DAQProcess Issue", 
		   ERS_EMPTY
		   )
		    

ERS_DECLARE_ISSUE_BASE( DAQIssues, 
  			CommandOrderNotSpecified,
  			DAQProcessIssue,
  			"Command " << cmd << " does not have a specified propagation order ",
			ERS_EMPTY,
 			((std::string) cmd)
			)



ERS_DECLARE_ISSUE_BASE( DAQIssues, 
  			ModuleThrowUknown,
  			DAQProcessIssue,
  			"Module " << mod_name << " threw an unknown exception after command " << cmd, 
 			ERS_EMPTY, 
			((std::string) mod_name) ((std::string) cmd)
			)


ERS_DECLARE_ISSUE_BASE( DAQIssues, 
  			ModuleThowStd,
			DAQProcessIssue,
  			"Module " << mod_name << " threw an std::exception after command " << cmd, 
 			ERS_EMPTY, 
			((std::string ) mod_name) ((std::string) cmd)
			)



}


#endif // APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_DAQMODULES_DAQMODULEISSUES_HH_ 
