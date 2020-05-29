/**
 * @file DAQModule Issues definition
 *
 * This is the files that implements the some generic ERS issues that can be 
 * reported by a DAQModule
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_DAQMODULES_DAQMODULEISSUES_HH_
#define APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_DAQMODULES_DAQMODULEISSUES_HH_

#include <string>

#include <ers/Issue.h>

namespace appframework {

ERS_DECLARE_ISSUE( DAQIssues, 
		   GeneralDAQModuleIssue, 
		   "General DAQModule Issue", 
		   ERS_EMPTY
		   )
		    

ERS_DECLARE_ISSUE_BASE( DAQIssues, 
  			UnknownCommand,
  			GeneralDAQModuleIssue,
  			"Command " << cmd << " is not recognised", 
			ERS_EMPTY,
 			((std::string) cmd)
			)


ERS_DECLARE_ISSUE_BASE( DAQIssues, 
  			CommandFailed,
  			GeneralDAQModuleIssue,
  			"Command " << cmd << " failed to execute for reason " << reason, 
 			ERS_EMPTY, 
			((std::string ) cmd) ((std::string) reason)
			)



}


#endif // APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_DAQMODULES_DAQMODULEISSUES_HH_
