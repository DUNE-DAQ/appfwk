/**
 * @file State Machine Data Structures
 *
 * Defines a set of data structures related to the state machine/transitions
 *
 * This is part of the DUNE DAQ Application Framework, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have received with this code.
 */

#ifndef app_framework_base_Core_StateMachine_hh
#define app_framework_base_Core_StateMachine_hh

namespace appframework {
/**
 * @brief Named transitions
 */
enum class TransitionName {
    Start, ///< The Start transition
    Stop, ///< The Stop transition
};

/**
 * @brief States that UserModules and/or DAQProcess can be in
 */
enum class StateMachineState {
    Initial, ///< Initial state
    Booted, ///< Started up
    ModulesRegistered,  ///< Modules loaded (modules should be in Booted state)
    Configured, ///< Configuration loaded
    Running, ///< Started
};
}  // namespace appframework

#endif  // app_framework_base_Core_StateMachine_hh