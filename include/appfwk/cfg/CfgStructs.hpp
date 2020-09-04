/*
 * This file is 100% generated.  Any manual edits will likely be lost.
 */
#ifndef DUNEDAQ__APPFWK__APP_STRUCTS_HPP
#define DUNEDAQ__APPFWK__APP_STRUCTS_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace dunedaq::appfwk::cfg {

    // @brief 
    using Data = nlohmann::json;

    /// @brief Type/instance name pair for factory construction
    struct TypeName {
        std::string type{  };
        std::string name{  };
    };


    /// @brief Data addressed to a particular instance
    struct AddrDat {
        TypeName tn{  };
        Data data{  };
    };


    /// @brief Addressed command data
    struct Addressed {
        std::vector<AddrDat> addrdats{  };
    };


} // namespace dunedaq::appfwk::cfg

#endif // DUNEDAQ__APPFWK__APP_STRUCTS_HPP