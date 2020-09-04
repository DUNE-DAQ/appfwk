/*
 * This file is 100% generated.  Any manual edits will likely be lost.
 */
#ifndef DUNEDAQ__APPFWK__CMD_NLJS_HPP
#define DUNEDAQ__APPFWK__CMD_NLJS_HPP

#include "appfwk/cmd/CmdStructs.hpp"

#include <nlohmann/json.hpp>


namespace dunedaq::appfwk::cmd {

    using json = nlohmann::json;

    // Converters for Command
    // A command
    inline void to_json(json& j, const Command& obj) {
        j["id"] = obj.id;
        j["data"] = obj.data;
    }

    inline void from_json(const json& j, Command& obj) {
        if (j.contains("id")) {
            obj.id = j.at("id");
        }

        if (j.contains("data")) {
            obj.data = j.at("data");
        }
    }


} // namespace dunedaq::appfwk::cmd

#endif // DUNEDAQ__APPFWK__CMD_NLJS_HPP