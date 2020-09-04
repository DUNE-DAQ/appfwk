/*
 * This file is 100% generated.  Any manual edits will likely be lost.
 */
#ifndef DUNEDAQ__APPFWK__APP_NLJS_HPP
#define DUNEDAQ__APPFWK__APP_NLJS_HPP

#include "appfwk/cfg/CfgStructs.hpp"

#include <nlohmann/json.hpp>


namespace dunedaq::appfwk::cfg {

    using json = nlohmann::json;

    // Converters for TypeName
    // Type/instance name pair for factory construction
    inline void to_json(json& j, const TypeName& obj) {
        j["type"] = obj.type;
        j["name"] = obj.name;
    }
    inline void from_json(const json& j, TypeName& obj) {
        if (j.contains("type")) {
            j.at("type").get_to(obj.type);
        }
        if (j.contains("name")) {
            j.at("name").get_to(obj.name);
        }
    }
    // Converters for AddrDat
    // Data addressed to a particular instance
    inline void to_json(json& j, const AddrDat& obj) {
        j["tn"] = obj.tn;
        j["data"] = obj.data;
    }
    inline void from_json(const json& j, AddrDat& obj) {
        if (j.contains("tn")) {
            j.at("tn").get_to(obj.tn);
        }
        if (j.contains("data")) {
            obj.data = j.at("data");
        }
    }
    // Converters for Addressed
    // Addressed command data
    inline void to_json(json& j, const Addressed& obj) {
        j["addrdats"] = obj.addrdats;
    }
    inline void from_json(const json& j, Addressed& obj) {
        if (j.contains("addrdats")) {
            j.at("addrdats").get_to(obj.addrdats);
        }
    }

} // namespace dunedaq::appfwk::app

#endif // DUNEDAQ__APPFWK__APP_NLJS_HPP