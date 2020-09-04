/*
 * This file is 100% generated.  Any manual edits will likely be lost.
 */
#ifndef DUNEDAQ__APPFWK__DQM_STRUCTS_HPP
#define DUNEDAQ__APPFWK__DQM_STRUCTS_HPP

#include <nlohmann/json.hpp>
#include <map>
#include <string>

// #include "appfwk/CmdStructs.hpp"

namespace dunedaq::appfwk::cfg {

    using namespace dunedaq::appfwk::cmd;

    /// @brief The kinds of queues
    enum class Kind: unsigned {
        Unknown,
        StdDeQueue,
        FollySPSCQueue,
        FollyMPMCQueue,
    };
    struct KindNames {
        static constexpr char const* Unknown = "Unknown";
        static constexpr char const* StdDeQueue = "StdDeQueue";
        static constexpr char const* FollySPSCQueue = "FollySPSCQueue";
        static constexpr char const* FollyMPMCQueue = "FollyMPMCQueue";
    };
    /// Convert a Kind to a string.
    inline
    std::string str(const Kind& kind) {
        const std::map<Kind, std::string> kind2s = {
            { Kind::Unknown, "Unknown" },
            { Kind::StdDeQueue, "StdDeQueue" },
            { Kind::FollySPSCQueue, "FollySPSCQueue" },
            { Kind::FollyMPMCQueue, "FollyMPMCQueue" },
        };
        auto it = kind2s.find(kind);
        if (it == kind2s.end()) {
            // fixme: replace with ERS
            throw std::runtime_error("unknown Kind enum");
        }
        return it->second;
    }
    /// Convert a string to a Kind
    inline
    Kind toKind(const std::string& kind) {
        const std::map<std::string, Kind> s2kind = {
            { "Unknown", Kind::Unknown },
            { "StdDeQueue", Kind::StdDeQueue },
            { "FollySPSCQueue", Kind::FollySPSCQueue },
            { "FollyMPMCQueue", Kind::FollyMPMCQueue },
        };
        auto it = s2kind.find(kind);
        if (it == s2kind.end()) {
            throw std::runtime_error("no Kind enum for " + kind);
        }
        return it->second;
    }

    // @brief 
    using Data = nlohmann::json;

    /// @brief Queue 'init' data structure
    struct QueueInit {
        Kind kind{  };
        uint64_t capacity{  };
    };


    /// @brief Module 'init' data structure
    struct ModInit {
        std::string plugin{  };
        Data data{  };
    };


} // namespace dunedaq::appfwk::cfg

#endif // DUNEDAQ__APPFWK__DQM_STRUCTS_HPP