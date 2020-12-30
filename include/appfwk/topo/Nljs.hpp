/*
 * This file is 100% generated.  Any manual edits will likely be lost.
 *
 * This contains functions struct and other type definitions for shema in 
 * namespace dunedaq::appfwk::topo to be serialized via nlohmann::json.
 */
#ifndef DUNEDAQ_APPFWK_TOPO_NLJS_HPP
#define DUNEDAQ_APPFWK_TOPO_NLJS_HPP

// My structs
#include "appfwk/topo/Structs.hpp"

// Nljs for externally referenced schema
#include "cmdlib/cmd/Nljs.hpp"

#include <nlohmann/json.hpp>

namespace dunedaq::appfwk::topo {

    using data_t = nlohmann::json;    NLOHMANN_JSON_SERIALIZE_ENUM( QueueKind, {
            { dunedaq::appfwk::topo::QueueKind::Unknown, "Unknown" },
            { dunedaq::appfwk::topo::QueueKind::StdDeQueue, "StdDeQueue" },
            { dunedaq::appfwk::topo::QueueKind::FollySPSCQueue, "FollySPSCQueue" },
            { dunedaq::appfwk::topo::QueueKind::FollyMPMCQueue, "FollyMPMCQueue" },
        })

    
    inline void to_json(data_t& j, const QueueSpec& obj) {
        j["kind"] = obj.kind;
        j["inst"] = obj.inst;
        j["capacity"] = obj.capacity;
    }
    
    inline void from_json(const data_t& j, QueueSpec& obj) {
        if (j.contains("kind"))
            j.at("kind").get_to(obj.kind);    
        if (j.contains("inst"))
            j.at("inst").get_to(obj.inst);    
        if (j.contains("capacity"))
            j.at("capacity").get_to(obj.capacity);    
    }
    
    inline void to_json(data_t& j, const ModSpec& obj) {
        j["plugin"] = obj.plugin;
        j["inst"] = obj.inst;
        j["data"] = obj.data;
    }
    
    inline void from_json(const data_t& j, ModSpec& obj) {
        if (j.contains("plugin"))
            j.at("plugin").get_to(obj.plugin);    
        if (j.contains("inst"))
            j.at("inst").get_to(obj.inst);    
        if (j.contains("data"))
            j.at("data").get_to(obj.data);    
    }
    
    inline void to_json(data_t& j, const Init& obj) {
        j["queues"] = obj.queues;
        j["modules"] = obj.modules;
    }
    
    inline void from_json(const data_t& j, Init& obj) {
        if (j.contains("queues"))
            j.at("queues").get_to(obj.queues);    
        if (j.contains("modules"))
            j.at("modules").get_to(obj.modules);    
    }
    
    inline void to_json(data_t& j, const QueueInfo& obj) {
        j["inst"] = obj.inst;
        j["name"] = obj.name;
        j["dir"] = obj.dir;
    }
    
    inline void from_json(const data_t& j, QueueInfo& obj) {
        if (j.contains("inst"))
            j.at("inst").get_to(obj.inst);    
        if (j.contains("name"))
            j.at("name").get_to(obj.name);    
        if (j.contains("dir"))
            j.at("dir").get_to(obj.dir);    
    }
    
    inline void to_json(data_t& j, const ModInit& obj) {
        j["qinfos"] = obj.qinfos;
    }
    
    inline void from_json(const data_t& j, ModInit& obj) {
        if (j.contains("qinfos"))
            j.at("qinfos").get_to(obj.qinfos);    
    }
    
} // namespace dunedaq::appfwk::topo

#endif // DUNEDAQ_APPFWK_TOPO_NLJS_HPP