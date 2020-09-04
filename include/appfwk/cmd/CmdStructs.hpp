/*
 * This file is 100% generated.  Any manual edits will likely be lost.
 */
#ifndef DUNEDAQ__APPFWK__CMD_STRUCTS_HPP
#define DUNEDAQ__APPFWK__CMD_STRUCTS_HPP

#include <nlohmann/json.hpp>
#include <map>

namespace dunedaq::appfwk::cmd {

    // @brief 
    using Data = nlohmann::json;

    /// @brief A command
    struct Command {
        std::string id{  };
        Data data{  };
    };

} // namespace dunedaq::appfwk::cmd

#endif // DUNEDAQ__APPFWK__CMD_STRUCTS_HPP