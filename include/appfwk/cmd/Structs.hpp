/*
 * This file is 100% generated.  Any manual edits will likely be lost.
 *
 * This contains struct and other type definitions for shema in 
 * namespace dunedaq::appfwk::cmd.
 */
#ifndef DUNEDAQ_APPFWK_CMD_STRUCTS_HPP
#define DUNEDAQ_APPFWK_CMD_STRUCTS_HPP


#include <nlohmann/json.hpp>
#include <string>

namespace dunedaq::appfwk::cmd {

    // @brief The command name.  FIXME: this should be an enum!
    using Name = std::string;

    // @brief An opaque object holding lower layer substructure
    using Data = nlohmann::json;

    // @brief Top-level command object structure
    struct Command {

        // @brief Identify the type of command
        Name id;

        // @brief Command data object with type-specific structure
        Data data;
    };

} // namespace dunedaq::appfwk::cmd

#endif // DUNEDAQ_APPFWK_CMD_STRUCTS_HPP