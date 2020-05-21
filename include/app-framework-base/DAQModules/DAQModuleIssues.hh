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

ERS_DECLARE_ISSUE( appframework, 
		   DAQModuleIssue, 
		   "Generic DAQModule Issue encoutered" 
		   )
		    

ERS_DECLARE_ISSUE_BASE( appframework, 
			UnknownCommand,
			"Command " << cmd << " is not recognised", 
			( const std::string & ) cmd 
			)





#endif // APP_FRAMEWORK_BASE_INCLUDE_APP_FRAMEWORK_BASE_DAQMODULES_DAQMODULEISSUES_HH_
