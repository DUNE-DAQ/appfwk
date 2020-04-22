#include <app-framework/CommandIssues.hh>

ERS_DEFINE_ISSUE_CXX( 	appframework, 			// namespace
			CommandNotRegisted, 			// issue class name
            "Command '" << command_name
                    << "' does not have an entry in the CommandOrderMap! UserModules will receive this command in an unspecified order!",
                ((const char *)command_name ) // single attribute 
            )