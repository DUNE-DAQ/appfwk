/*
 * This file is 100% generated.  Any manual edits will likely be lost.
 *
 * This contains struct and other type definitions for shema in 
 * namespace dunedaq::appfwk::cmd.
 */
#ifndef DUNEDAQ_APPFWK_CMD_STRUCTS_HPP
#define DUNEDAQ_APPFWK_CMD_STRUCTS_HPP

#include <cstdint>

#include <nlohmann/json.hpp>
#include <vector>
#include <string>

namespace dunedaq::appfwk::cmd {

    // @brief String used as a regex match
    using Match = std::string;

    // @brief An opaque object holding lower layer substructure
    using Data = nlohmann::json;

    // @brief General, non-init module-level command data structure
    struct AddressedCmd {

        // @brief A regex that matches on module instance names
        Match match;

        // @brief The module-level command data object
        Data data;
    };

    // @brief A sequence of AddressedCmd
    using AddressedCmds = std::vector<dunedaq::appfwk::cmd::AddressedCmd>;

    // @brief The command name.  FIXME: this should be an enum!
    using CmdId = std::string;

    // @brief Structure of app-level, non-init command object
    struct CmdObj {

        // @brief Addressed, module command objects
        AddressedCmds modules;
    };

    // @brief Top-level command object structure
    struct Command {

        // @brief Identify the type of command
        CmdId id;

        // @brief Command data object with type-specific structure
        Data data;
    };

    // @brief The kinds (types/classes) of queues
    enum class QueueKind: unsigned {
        Unknown,
        StdDeQueue,
        FollySPSCQueue,
        FollyMPMCQueue,
    };

    // @brief Name of a target instance of a kind
    using InstName = std::string;

    // @brief Capacity of a queue
    using QueueCapacity = uint64_t;

    // @brief Queue specification
    struct QueueSpec {

        // @brief The kind (type) of queue
        QueueKind kind;

        // @brief Instance name
        InstName inst;

        // @brief The queue capacity
        QueueCapacity capacity;
    };

    // @brief A sequence of QueueSpec
    using QueueSpecs = std::vector<dunedaq::appfwk::cmd::QueueSpec>;

    // @brief Name of a plugin
    using PluginName = std::string;

    // @brief Module specification
    struct ModSpec {

        // @brief Name of a plugin providing the module
        PluginName plugin;

        // @brief Module instance name
        InstName inst;

        // @brief Specific to the module implementation
        Data data;
    };

    // @brief A sequence of ModSpec
    using ModSpecs = std::vector<dunedaq::appfwk::cmd::ModSpec>;

    // @brief The app-level init command data object struction
    struct Init {

        // @brief Initial Queue specifications
        QueueSpecs queues;

        // @brief Initial Module specifications
        ModSpecs modules;
    };

    // @brief A label hard-wired into code
    using Label = std::string;

    // @brief Direction of queue from the point of view of a module holding an endpoint
    using QueueDir = std::string;

    // @brief Information for a module to find a Queue
    struct QueueInfo {

        // @brief The queue instance name
        InstName inst;

        // @brief The name by which this queue is known to the module
        Label name;

        // @brief The direction of the queue as seen by the module
        QueueDir dir;
    };

    // @brief A sequence of QueueInfo
    using QueueInfos = std::vector<dunedaq::appfwk::cmd::QueueInfo>;

    // @brief A standardized portion of every ModSpec.data
    struct ModInit {

        // @brief 
        QueueInfos qinfos;
    };

} // namespace dunedaq::appfwk::cmd

#endif // DUNEDAQ_APPFWK_CMD_STRUCTS_HPP