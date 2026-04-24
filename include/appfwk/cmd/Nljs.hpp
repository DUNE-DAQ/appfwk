/*
 * This file is 100% generated.  Any manual edits will likely be lost.
 *
 * This contains functions struct and other type definitions for shema in 
 * namespace dunedaq::appfwk::cmd to be serialized via nlohmann::json.
 */
#ifndef DUNEDAQ_APPFWK_CMD_NLJS_HPP
#define DUNEDAQ_APPFWK_CMD_NLJS_HPP

// My structs
#include "appfwk/cmd/Structs.hpp"

// Nljs for externally referenced schema
#include "cmdlib/cmd/Nljs.hpp"

#include <nlohmann/json.hpp>

namespace dunedaq::appfwk::cmd {

    using data_t = nlohmann::json;
    
    inline void to_json(data_t& j, const AddressedCmd& obj) {
        j["match"] = obj.match;
        j["data"] = obj.data;
    }
    
    inline void from_json(const data_t& j, AddressedCmd& obj) {
        if (j.contains("match"))
            j.at("match").get_to(obj.match);    
        if (j.contains("data"))
            j.at("data").get_to(obj.data);    
    }
    
    inline void to_json(data_t& j, const CmdObj& obj) {
        j["modules"] = obj.modules;
    }
    
    inline void from_json(const data_t& j, CmdObj& obj) {
        if (j.contains("modules"))
            j.at("modules").get_to(obj.modules);    
    }
    
} // namespace dunedaq::appfwk::cmd

#endif // DUNEDAQ_APPFWK_CMD_NLJS_HPP