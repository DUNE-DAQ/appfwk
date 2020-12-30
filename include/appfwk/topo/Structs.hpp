/*
 * This file is 100% generated.  Any manual edits will likely be lost.
 *
 * This contains struct and other type definitions for shema in 
 * namespace dunedaq::appfwk::topo.
 */
#ifndef DUNEDAQ_APPFWK_TOPO_STRUCTS_HPP
#define DUNEDAQ_APPFWK_TOPO_STRUCTS_HPP

#include <cstdint>
#include "cmdlib/cmd/Structs.hpp"

#include <vector>
#include <string>

namespace dunedaq::appfwk::topo {

    // @brief The kinds (types/classes) of queues
    enum class QueueKind: unsigned {
        Unknown,
        StdDeQueue,
        FollySPSCQueue,
        FollyMPMCQueue,
    };
    // return a string representation of a QueueKind.
    inline
    const char* str(QueueKind val) {
        if (val == QueueKind::Unknown) { return "Unknown" ;}
        if (val == QueueKind::StdDeQueue) { return "StdDeQueue" ;}
        if (val == QueueKind::FollySPSCQueue) { return "FollySPSCQueue" ;}
        if (val == QueueKind::FollyMPMCQueue) { return "FollyMPMCQueue" ;}
        return "";                  // should not reach
    }
    inline
    QueueKind parse_QueueKind(std::string val, QueueKind def = QueueKind::Unknown) {
        if (val == "Unknown") { return QueueKind::Unknown; }
        if (val == "StdDeQueue") { return QueueKind::StdDeQueue; }
        if (val == "FollySPSCQueue") { return QueueKind::FollySPSCQueue; }
        if (val == "FollyMPMCQueue") { return QueueKind::FollyMPMCQueue; }
        return def;
    }

    // @brief Name of a target instance of a kind
    using InstName = std::string;

    // @brief Capacity of a queue
    using QueueCapacity = uint64_t; // NOLINT


    // @brief Queue specification
    struct QueueSpec {

        // @brief The kind (type) of queue
        dunedaq::appfwk::topo::QueueKind kind = dunedaq::appfwk::topo::QueueKind::Unknown;

        // @brief Instance name
        dunedaq::appfwk::topo::InstName inst = "";

        // @brief The queue capacity
        dunedaq::appfwk::topo::QueueCapacity capacity = 0;
    };

    // @brief A sequence of QueueSpec
    using QueueSpecs = std::vector<dunedaq::appfwk::topo::QueueSpec>;

    // @brief Name of a plugin
    using PluginName = std::string;

    // @brief Module specification
    struct ModSpec {

        // @brief Name of a plugin providing the module
        dunedaq::appfwk::topo::PluginName plugin = "";

        // @brief Module instance name
        dunedaq::appfwk::topo::InstName inst = "";

        // @brief Specific to the module implementation
        dunedaq::cmdlib::cmd::Data data = {};
    };

    // @brief A sequence of ModSpec
    using ModSpecs = std::vector<dunedaq::appfwk::topo::ModSpec>;

    // @brief The app-level init command data object struction
    struct Init {

        // @brief Initial Queue specifications
        dunedaq::appfwk::topo::QueueSpecs queues = {};

        // @brief Initial Module specifications
        dunedaq::appfwk::topo::ModSpecs modules = {};
    };

    // @brief A label hard-wired into code
    using Label = std::string;

    // @brief Direction of queue from the point of view of a module holding an endpoint
    using QueueDir = std::string;

    // @brief Information for a module to find a Queue
    struct QueueInfo {

        // @brief The queue instance name
        dunedaq::appfwk::topo::InstName inst = "";

        // @brief The name by which this queue is known to the module
        dunedaq::appfwk::topo::Label name = "";

        // @brief The direction of the queue as seen by the module
        dunedaq::appfwk::topo::QueueDir dir = "";
    };

    // @brief A sequence of QueueInfo
    using QueueInfos = std::vector<dunedaq::appfwk::topo::QueueInfo>;

    // @brief A standardized portion of every ModSpec.data
    struct ModInit {

        // @brief Information for a module to find its queue
        dunedaq::appfwk::topo::QueueInfos qinfos = {};
    };

} // namespace dunedaq::appfwk::topo

#endif // DUNEDAQ_APPFWK_TOPO_STRUCTS_HPP