/**
 * @file QueryResponseCommandFacility.cpp QueryResponseCommandFacility class
 * Implementation
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "QueryResponseCommandFacility.hpp"

#include <iostream>
#include <string>

namespace dunedaq::appfwk {

int
QueryResponseCommandFacility::listen(const DAQProcess& process) const
{
  try {
    bool keepGoing = true;
    while (keepGoing) {
      std::cout << "Enter a command" << std::endl; // NOLINT
      std::string comm;
      std::getline(std::cin, comm);

      if (comm == "quit") {
        keepGoing = false;
        break;
      } else {
        process.execute_command(comm);
      }
    }
  } catch (...) // NOLINT
  {
    return -1;
  }

  return 0;
}

QueryResponseCommandFacility::~QueryResponseCommandFacility() = default;
} // namespace dunedaq::appfwk

DEFINE_DUNE_COMMAND_FACILITY(dunedaq::appfwk::QueryResponseCommandFacility)
