#ifndef app_framework_base_Core_StateMachine_hh
#define app_framework_base_Core_StateMachine_hh

namespace appframework {
enum class TransitionName {
    Start,
    Stop,
};

enum class StateMachineState {
    Initial,
    Booted,
    ModulesRegistered,  ///< How granular do we need to be?
    Configured,
    Running,
};
}  // namespace appframework

#endif  // app_framework_base_Core_StateMachine_hh