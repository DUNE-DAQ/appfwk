/*
 * This file is 100% generated.  Any manual edits will likely be lost.
 *
 * This contains struct and other type definitions for shema in 
 * namespace dunedaq::appfwk::app.
 */
#ifndef DUNEDAQ_APPFWK_APP_STRUCTS_HPP
#define DUNEDAQ_APPFWK_APP_STRUCTS_HPP


#include <nlohmann/json.hpp>
#include <vector>
#include <string>

namespace dunedaq::appfwk::app {

    // @brief Name of a kind (class/type) of target instance
    using KindName = std::string;

    // @brief Associate a kind and an instance name
    struct KindInst {

        // @brief A kind (class/type) name
        KindName kind;

        // @brief An instance name
        KindName inst;
    };

    // @brief An opaque object holding lower layer substructure
    using Data = nlohmann::json;

    // @brief Address a data object to a kind of instance
    struct AddrDat {

        // @brief The addressed instance
        KindInst ki;

        // @brief The data object for the instance
        Data data;
    };

    // @brief A sequence of AddrDat
    using AddrDats = std::vector<dunedaq::appfwk::app::AddrDat>;

    // @brief The app level data object
    struct Addressed {

        // @brief The data objects addressed to kinds of instances
        AddrDats addrdats;
    };

    // @brief Name of a target instance of a kind
    using InstName = std::string;

    // @brief A sequence of KindInst
    using KindInsts = std::vector<dunedaq::appfwk::app::KindInst>;

    // @brief Name of a plugin
    using PluginName = std::string;

    // @brief Module initialization data structure
    struct ModInit {

        // @brief Module plugin name
        PluginName plugin;

        // @brief Instance specific data
        Data data;
    };

    // @brief Capacity of a queue
    using QueueCapacity = uint64_t;

    // @brief The kinds (types/classes) of queues
    enum class QueueKind: unsigned {
        Unknown,
        StdDeQueue,
        FollySPSCQueue,
        FollyMPMCQueue,
    };

    // @brief Queue initialization data structure
    struct QueueInit {

        // @brief The kind of queue
        QueueKind kind;

        // @brief The queue capacity
        QueueCapacity capacity;
    };

} // namespace dunedaq::appfwk::app

#endif // DUNEDAQ_APPFWK_APP_STRUCTS_HPP