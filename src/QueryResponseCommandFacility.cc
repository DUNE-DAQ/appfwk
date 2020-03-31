/**
 * @file QueryResponseCommandFacility class Implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#include "app-framework/QueryResponseCommandFacility.hh"

#include <iostream>

namespace appframework {

int QueryResponseCommandFacility::listen(DAQProcess* process) {
    try {
        bool keepGoing = true;
        while (keepGoing) {
            std::cout << "Enter a command" << std::endl;
            std::string comm;
            std::getline(std::cin, comm);

            if (comm == "quit") {
                keepGoing = false;
                break;
            } else {
                process->execute_transition(stringToTransitionName(comm));
            }
        }
    } catch (...) {
        return -1;
    }

    return 0;
}

TransitionName QueryResponseCommandFacility::stringToTransitionName(std::string string) {
    if (string.find("stop") != std::string::npos || string.find("Stop") != std::string::npos) {
        return TransitionName::Stop;
    }
    return TransitionName::Start;
}
}  // namespace appframework