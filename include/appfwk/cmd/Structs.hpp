/*
 * This file is 100% generated.  Any manual edits will likely be lost.
 *
 * This contains struct and other type definitions for shema in 
 * namespace dunedaq::appfwk::cmd.
 */
#ifndef DUNEDAQ_APPFWK_CMD_STRUCTS_HPP
#define DUNEDAQ_APPFWK_CMD_STRUCTS_HPP

#include <cstdint>
#include "cmdlib/cmd/Structs.hpp"

#include <vector>
#include <string>

namespace dunedaq::appfwk::cmd {

    // @brief String used as a regex match
    using Match = std::string;

    // @brief General, non-init module-level command data structure
    struct AddressedCmd 
    {

        // @brief A regex that matches on module instance names
        Match match = "";

        // @brief The module-level command data object
        cmdlib::cmd::Data data = {};
    };

    // @brief A sequence of AddressedCmd
    using AddressedCmds = std::vector<dunedaq::appfwk::cmd::AddressedCmd>;

    // @brief Structure of app-level, non-init command object
    struct CmdObj 
    {

        // @brief Addressed, module command objects
        AddressedCmds modules = {};
    };

} // namespace dunedaq::appfwk::cmd

#endif // DUNEDAQ_APPFWK_CMD_STRUCTS_HPP