/*
 * This file is 100% generated.  Any manual edits will likely be lost.
 */
#ifndef DUNEDAQ__APPFWK__DQM_NLJS_HPP
#define DUNEDAQ__APPFWK__DQM_NLJS_HPP

#include "appfwk/cfg/DqmStructs.hpp"
#include "appfwk/cmd/CmdNljs.hpp"

#include <nlohmann/json.hpp>


namespace dunedaq::appfwk::cfg {

    using json = nlohmann::json;

    NLOHMANN_JSON_SERIALIZE_ENUM( Kind, {
            { Kind::Unknown, "Unknown" },
            { Kind::StdDeQueue, "StdDeQueue" },
            { Kind::FollySPSCQueue, "FollySPSCQueue" },
            { Kind::FollyMPMCQueue, "FollyMPMCQueue" },
        })

    // Converters for QueueInit
    // Queue 'init' data structure
    inline void to_json(json& j, const QueueInit& obj) {
        j["kind"] = obj.kind;
        j["capacity"] = obj.capacity;
    }
    inline void from_json(const json& j, QueueInit& obj) {
        if (j.contains("kind")) {
            j.at("kind").get_to(obj.kind);
        }
        if (j.contains("capacity")) {
            j.at("capacity").get_to(obj.capacity);
        }
    }
    // Converters for ModInit
    // Module 'init' data structure
    inline void to_json(json& j, const ModInit& obj) {
        j["plugin"] = obj.plugin;
        j["data"] = obj.data;
    }
    inline void from_json(const json& j, ModInit& obj) {
        if (j.contains("plugin")) {
            j.at("plugin").get_to(obj.plugin);
        }
        if (j.contains("data")) {
            obj.data = j.at("data");
        }
    }


} // namespace dunedaq::appfwk::cfg

#endif // DUNEDAQ__APPFWK__DQM_NLJS_HPP