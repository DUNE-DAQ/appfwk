#ifndef APP_FRAMEWORK_COMMANDISSUES_HH_
#define APP_FRAMEWORK_COMMANDISSUES_HH_

#include <ers/Issue.h>

/** \def ers::File This is the base class for all file related issues. 
 */
ERS_DECLARE_ISSUE_HPP( 	appframework, 			// namespace
			CommandNotRegisted, 			// issue class name
            "Command '" << command_name
                    << "' does not have an entry in the CommandOrderMap! UserModules will receive this command in an unspecified order!",
                ((const char *)command_name ) // single attribute 
            )


#endif  // APP_FRAMEWORK_COMMANDISSUES_HH_
