/*
 * This file is 100% generated.  Any manual edits will likely be lost.
 *
 * This contains functions struct and other type definitions for shema in 
 * namespace dunedaq::appfwk::app to be serialized via nlohmann::json.
 */
#ifndef DUNEDAQ_APPFWK_APP_NLJS_HPP
#define DUNEDAQ_APPFWK_APP_NLJS_HPP


#include "appfwk/app/Structs.hpp"


#include <nlohmann/json.hpp>

namespace dunedaq::appfwk::app {

    using data_t = nlohmann::json;

    NLOHMANN_JSON_SERIALIZE_ENUM( QueueKind, {
            { dunedaq::appfwk::app::QueueKind::Unknown, "Unknown" },
            { dunedaq::appfwk::app::QueueKind::StdDeQueue, "StdDeQueue" },
            { dunedaq::appfwk::app::QueueKind::FollySPSCQueue, "FollySPSCQueue" },
            { dunedaq::appfwk::app::QueueKind::FollyMPMCQueue, "FollyMPMCQueue" },
        })

    
    inline void to_json(data_t& j, const KindInst& obj) {
        j["kind"] = obj.kind;
        j["inst"] = obj.inst;
    }
    
    inline void from_json(const data_t& j, KindInst& obj) {
        if (j.contains("kind"))
            j.at("kind").get_to(obj.kind);    
        if (j.contains("inst"))
            j.at("inst").get_to(obj.inst);    
    }
    
    inline void to_json(data_t& j, const AddrDat& obj) {
        j["ki"] = obj.ki;
        j["data"] = obj.data;
    }
    
    inline void from_json(const data_t& j, AddrDat& obj) {
        if (j.contains("ki"))
            j.at("ki").get_to(obj.ki);    
        obj.data = j.at("data");
    }
    
    inline void to_json(data_t& j, const Addressed& obj) {
        j["addrdats"] = obj.addrdats;
    }
    
    inline void from_json(const data_t& j, Addressed& obj) {
        if (j.contains("addrdats"))
            j.at("addrdats").get_to(obj.addrdats);    
    }
    
    inline void to_json(data_t& j, const ModInit& obj) {
        j["plugin"] = obj.plugin;
        j["data"] = obj.data;
    }
    
    inline void from_json(const data_t& j, ModInit& obj) {
        if (j.contains("plugin"))
            j.at("plugin").get_to(obj.plugin);    
        obj.data = j.at("data");
    }
    
    inline void to_json(data_t& j, const QueueInit& obj) {
        j["kind"] = obj.kind;
        j["capacity"] = obj.capacity;
    }
    
    inline void from_json(const data_t& j, QueueInit& obj) {
        if (j.contains("kind"))
            j.at("kind").get_to(obj.kind);    
        if (j.contains("capacity"))
            j.at("capacity").get_to(obj.capacity);    
    }
    
    // fixme: add support for MessagePack serializers (at least)

} // namespace dunedaq::appfwk::app

#endif // DUNEDAQ_APPFWK_APP_NLJS_HPP